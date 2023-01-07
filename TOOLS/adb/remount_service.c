/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mount.h>
#include <errno.h>

#include "sysdeps.h"

#define  TRACE_TAG  TRACE_ADB
#include "adb.h"


static int system_ro = 1;

/* Returns the device used to mount a directory in /proc/mounts */
static char *find_mount(const char *dir)
{
    int fd;
    int res;
    int size;
    char *token = NULL;
    const char delims[] = "\n";
    char buf[4096];

    printf("<===>find_mount enter \n");	

    fd = unix_open("/proc/mounts", O_RDONLY);
    if (fd < 0){
	 printf("<===>find_mount unix_open fail \n");
        return NULL;
    	}

	

    buf[sizeof(buf) - 1] = '\0';
    size = adb_read(fd, buf, sizeof(buf) - 1);
    adb_close(fd);

    printf("<===>find_mount buf=%s \n", buf);	

    token = strtok(buf, delims);

    printf("<===>find_mount token=%s \n", token);		

    while (token) {
        char mount_dev[256];
        char mount_dir[256];
        int mount_freq;
        int mount_passno;

        res = sscanf(token, "%255s %255s %*s %*s %d %d\n",
                     mount_dev, mount_dir, &mount_freq, &mount_passno);
	 printf("<===>find_mount token=%s mount_dev=%s mount_dir=%s mount_freq=%d mount_passno=%d \n", token, mount_dev, mount_dir, mount_freq, mount_passno);		
        mount_dev[255] = 0;
        mount_dir[255] = 0;
        if (res == 4 && (strcmp(dir, mount_dir) == 0)){
              printf("<===>find_mount return=%s \n", strdup(mount_dev));		
		return strdup(mount_dev);
        	}	

        token = strtok(NULL, delims);
    }

    printf("<===>find_mount end \n");
    return NULL;
}

/* Init mounts /system as read only, remount to enable writes. */
static int remount_system()
{
    char *dev;

    if (system_ro == 0) {
	printf(" <===>remount_system system_ro == 0 \n");
        return 0;
    }

    printf(" <===>remount_system bengin to  find_remount \n");
    dev = find_mount("/system");
    //dev = find_mount("/");

    if (!dev)
	 printf("<===>remount_system find_remount result=%s \n", dev);
        return -1;

    system_ro = mount(dev, "/system", "none", MS_REMOUNT, NULL);
    //system_ro = mount(dev, "/", "none", MS_REMOUNT, NULL);

    free(dev);

   printf("<===>remount_system end \n");
    return system_ro;
}

static void write_string(int fd, const char* str)
{
    writex(fd, str, strlen(str));
}

void remount_service(int fd, void *cookie)
{
    int ret = remount_system();

    if (!ret)
       write_string(fd, "remount succeeded\n");
    else {
        char    buffer[200];
        snprintf(buffer, sizeof(buffer), "remount failed: %s\n", strerror(errno));
        write_string(fd, buffer);
    }

    adb_close(fd);
}

