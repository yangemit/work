#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<net/if.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include "camera_control.h"

static unsigned int do_get_camera_version(unsigned char  *buf, unsigned int bufLength, unsigned int cmdtype)
{
    printf("do_get_camera_version\n");
    int bufferLength = 32;
    memcpy_s(buf, CAMERA_VERSION_STRING_LEN, CAMERA_VERSION, CAMERA_VERSION_STRING_LEN);
    return bufferLength;
}


extern unsigned char g_firmware_update;
static unsigned int do_switch_to_ufu_mode(unsigned char  *buf, unsigned int bufLength, unsigned int cmdtype)
{
	printf("do_switch_to_ufu\n");
	g_firmware_update=1;
    return 0;
}
 static unsigned int do_set_bitrate(unsigned char  *buf, unsigned int bufLength, unsigned int cmdtype)
{
    int s32Ret;
    int bitrate;
    bitrate= ((buf[0])+(buf[1]<<8));
    printf("do_set_bitrate data =%d kbps\n",bitrate);

    return 0;
}

 struct uvc_camera_cmd cmd_mappings[10] = {
     {
        .id = CMD_GET_CAMERA_VERSION,
        .name = "get camera version",
        .uvc_control_func = do_get_camera_version,
    },
    {
        .id = CMD_SET_UFU_MODE,
        .name = "switch ufu mode",
        .uvc_control_func = do_switch_to_ufu_mode,
    },
   {
        .id = CMD_SET_VIBRATE,
        .name = "Set  bitrate",
        .uvc_control_func = do_set_bitrate,
    },
};

 int run_cmd_func(unsigned char *buf, unsigned int bufLength, unsigned int cmdtype, unsigned int cmd_id)
{
    struct uvc_camera_cmd *cmd_cb;
    int ret = -1;

    if (cmd_id >= CMD_MAX_NUM) {
        return -1;
    }

    cmd_cb = &cmd_mappings[cmd_id];

    if (cmd_cb->uvc_control_func != NULL) {
        ret = cmd_cb->uvc_control_func (buf, bufLength, cmdtype);
    }

    return ret;
}


