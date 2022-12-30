/*********************************************************
 * File Name   : hsb.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2020-10-15 10:53
 ********************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "uart.h"

static struct uvc_hsb_string hsb_string[] = {
	/*
	 *{0,"null"},
	 *{MANUFACTURE, "manufact_lab"},
	 *{SREIALNUMBER, "serial_lab"},
	 *{VID, "vendor_id"},
	 *{PID, "product_id"},
	 *{BCD, "device_bcd"},
	 *{MODEL, "model_lab"},
	 *{CMEI, "cmei_lab"},
	 */
	{"null"},
	{"manufact_lab"},
	{"serial_lab"},
	{"vendor_id"},
	{"product_id"},
	{"device_bcd"},
	{"model_lab"},
	{"cmei_lab"},
};

int kiva_version_read(char *data, int *length)
{
	FILE *config_fd = NULL;
	char line_str[32] = {0};

	config_fd = fopen(KIVA_VERSION_PATH, "r+");
	if(config_fd == NULL){
		printf("open is version.txt is failed\n");
		return -1;
	}

	fseek(config_fd, 0L, SEEK_SET);

	if (fscanf(config_fd, "%[^\n]", line_str) < 0)
		printf("fscanf is faild\n");

	fclose(config_fd);

	if(strlen(line_str)){
		strcpy(data, line_str);
		printf("version=%s,data=%s\n",line_str,data);
		*length = strlen(line_str);
		/*cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_VALID, line_str, strlen(line_str));*/
		return 0;
	}
	return -1;
}


int sn_contrl_read(char *read_buf, int *data)
{
	char key[64] = {0};
	char date[64] = {0};
	char line_str[128] = {0};
	char attr_string[128] = {0};

	FILE *config_fd = NULL;
	strcpy(attr_string, hsb_string[2].s);

	/*
	 *hid_cmd_head *head = (hid_cmd_head*)read_buf;
	 *cmd_cur = head->cmd >> 4;
	 *index = sizeof(hsb_string) / sizeof(struct uvc_hsb_string);
	 *for(i = 0; i < index; i++){
	 *        if(cmd_cur == hsb_string[i].id){
	 *                strcpy(attr_string, hsb_string[i].s);
	 *                break;
	 *        }
	 *        else if(i == index - 1){
	 *                printf("not No corresponding option\n");
	 *                return -1;
	 *        }
	 *}
	 */

	config_fd = fopen(CONFIG_FILE, "rb+");
	if(config_fd == NULL){
		printf("open is uvc.config is failed\n");
		return -1;
	}
	/*
	 *fseek(config_fd, 0L, SEEK_END);
	 *printf("+++++++++++++++++++++config_fd if len =%d+++++++++++++++\n",ftell(config_fd));
	 */
	fseek(config_fd, 0L, SEEK_SET);

	while (!feof(config_fd)) {
		if (fscanf(config_fd, "%[^\n]", line_str) < 0){
			printf("fscanf is faild\n");
			break;
		}

		if (sscanf(line_str, "%[^:]:%[^\n]", key, date) != 2) {
			printf("warning: invalid param:%s\n", line_str);
			fseek(config_fd , 1L, SEEK_CUR);
			continue;
		}

		char *ch;
		ch = strchr(key,' ');
		if(ch)*ch = '\0';

		if(strcmp(key, attr_string) == 0){
			*data = strlen(date);
			strcpy(read_buf, date);
			/*cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_VALID, date, data_len);*/
			break;
		}
		fseek(config_fd , 1L, SEEK_CUR);
		if(feof(config_fd))
			return -1;
	}
	return 0;

}

int sn_contrl_write(char *sn_buf)
{
	int len_config,len_diff,len_mod;
	int ret = -1;
	char sn_info[64] = {0};
	char key[32] = {0};
	char line_str[96] = {0};

	char config_diff[96] = {0};
	char uvc_config_buf[1024] ={0};
	char attr_string[128] = {0};

	FILE *config_fd = NULL;
	strcpy(attr_string, hsb_string[2].s);
	memcpy(sn_info, sn_buf, strlen(sn_buf));

	config_fd = fopen(CONFIG_FILE, "rb+");
	if(config_fd == NULL){
		printf("open is uvc.config is failed\n");
		return -1;
	}
	fseek(config_fd, 0L, SEEK_END);
	len_config = ftell(config_fd);
	fseek(config_fd, 0L, SEEK_SET);

	while (!feof(config_fd)) {
		if (fscanf(config_fd, "%[^\n]", line_str) < 0){
			printf("fscanf is faild\n");
			break;
		}
		if (sscanf(line_str, "%[^:]", key) != 1) {
			printf("warning: invalid param:%s\n", line_str);
			fseek(config_fd , 1l, SEEK_CUR);
			continue;
		}

		char *ch;
		ch = strchr(key,' ');
		if(ch)*ch = '\0';

		if(strcmp(key, attr_string) == 0){
			len_diff = len_config -  ftell(config_fd);
			ret = fread(uvc_config_buf, 1, len_diff, config_fd);
			if (ret != len_diff)
				printf("fread is faild\n");

			fseek(config_fd, -(strlen(line_str) + len_diff), SEEK_CUR);
			fprintf(config_fd, "%s      :%s", attr_string, sn_info);
			fwrite(uvc_config_buf, 1, strlen(uvc_config_buf), config_fd);
			len_mod = ftell(config_fd);
			if(len_mod < len_config)
				fwrite(config_diff, 1, len_config - len_mod, config_fd);
			break;
		}
		fseek(config_fd , 1L, SEEK_CUR);
		if(feof(config_fd))
			return -1;
	}
	/*cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);*/
	fclose(config_fd);
	return 0;
}

