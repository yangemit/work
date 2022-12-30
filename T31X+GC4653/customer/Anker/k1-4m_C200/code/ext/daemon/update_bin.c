#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "update_bin.h"

int main(int argc, char** argv)
{
	int ret;

	if (argc < 2) {
		printf("usge: %s [iboot/kernel/appfs]\n", argv[0]);
		return -1;
	}

	char process_path[128];
	int len = readlink("/proc/self/exe", process_path, 128);
	if (len <= 0) {
		printf("ERROR: can't find my path! %s\n", strerror(errno));
		exit(1);
	}

	process_path[len] = '\0';

	/*update partition */
	char update_cmd[128];
	if (!strcmp(argv[1], "iboot")) {
		printf("update iboot.bin ...\n");
		if (!access(IBOOT_FIRMWARE_PATH, F_OK)) {
			sprintf(update_cmd, "flashcp -v %s /dev/mtd0", IBOOT_FIRMWARE_PATH);
			system(update_cmd);
			system("sync");
			sprintf(update_cmd, "rm %s", IBOOT_FIRMWARE_PATH);
			system(update_cmd);
			printf("update iboot.bin ...ok\n");
		} else {
			printf("%s not exist \n", IBOOT_FIRMWARE_PATH);
			return -1;
		}
	} else if (!strcmp(argv[1], "kernel")) {
		/*TODO:*/
		printf("update uImage...\n");
	} else if (!strcmp(argv[1], "appfs")) {
		/*TODO:*/
		printf("update appfs ...\n");
	} else {
		return -1;
	}

	/*delete self/exe */
	if (!access(process_path, F_OK)) {
		printf("%s \n", process_path);
		ret = unlink(process_path);
		if (ret) {
			printf("ERROR: can't remove anticopy file! %s\n", strerror(errno));
			exit(1);
		}
	}

	return 0;
}
