#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H
#define CAMERA_VERSION_STRING_LEN 20
#define CAMERA_VERSION  "V8-T31L3T02-LA1V000"
#define HW_VERSION "CMK-C1E-OT1729-V1.0"
enum CAMERA_CMD
{
    CMD_GET_CAMERA_VERSION = 0x01,
    CMD_SET_VIBRATE,
    CMD_SET_UFU_MODE,
    CMD_MAX_NUM
};

typedef unsigned int (*camera_control_func)(unsigned char *buf, unsigned int bufLength, unsigned int cmdtype);

struct uvc_camera_cmd {
    unsigned int id;
    unsigned char name[32];
    camera_control_func uvc_control_func;
};

extern int camera_register_cmd(struct uvc_camera_cmd *cmd);
extern int run_cmd_func(unsigned char  *buf, unsigned int bufLength, unsigned int cmdtype, unsigned int cmd_id);
#endif
