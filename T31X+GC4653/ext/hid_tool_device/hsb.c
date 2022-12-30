/*********************************************************
 * File Name   : hsb.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2020-10-15 10:53
 ********************************************************/
#include<stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "hid_test.h"

static struct uvc_hsb_string hsb_string[] = {
	{0,"null"},
	{MANUFACTURE, "manufact_lab"},
	{SREIALNUMBER, "serial_lab"},
	{PRODUCTNUMBER, "productnumber"},
	{VID, "vendor_id"},
	{PID, "product_id"},
	{BCD, "device_bcd"},
	{MODEL, "model_lab"},
	{CMEI, "cmei_lab"},
	{APPKEY, "appkey"},
};


static int sn_contrl_read(HSB_Object_Function *sn_function, char *read_buf)
{
	int i,index,cmd_cur;
	int data_len = 0;
	char key[64] = {0};
	char date[64] = {0};
	char line_str[128] = {0};
	char attr_string[128] = {0};

	FILE *config_fd = NULL;

	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	cmd_cur = head->cmd >> 4;
	index = sizeof(hsb_string) / sizeof(struct uvc_hsb_string);
	for(i = 0; i < index; i++){
		if(cmd_cur == hsb_string[i].id){
			strcpy(attr_string, hsb_string[i].s);
			break;
		}
		else if(i == index - 1){
			HID_WARNING("not No corresponding option\n");
			return -1;
		}
	}

	config_fd = fopen(sn_function->file_path, "rb+");
	if(config_fd == NULL){
		HID_ERR("open is uvc.config is failed\n");
		return -1;
	}
	/*
	 *fseek(config_fd, 0L, SEEK_END);
	 *HID_ERR("+++++++++++++++++++++config_fd if len =%d+++++++++++++++\n",ftell(config_fd));
	 */
	fseek(config_fd, 0L, SEEK_SET);

	while (!feof(config_fd)) {
		if (fscanf(config_fd, "%[^\n]", line_str) < 0){
			HID_WARNING("fscanf is faild\n");
			break;
		}

		if (sscanf(line_str, "%[^:]:%[^\n]", key, date) != 2) {
			HID_WARNING("warning: invalid param:%s\n", line_str);
			fseek(config_fd , 1L, SEEK_CUR);
			continue;
		}

		char *ch;
		ch = strchr(key,' ');
		if(ch)*ch = '\0';

		if(strcmp(key, attr_string) == 0){
			data_len = strlen(date);
			cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_VALID, date, data_len);
			break;
		}
		fseek(config_fd , 1L, SEEK_CUR);
		if(feof(config_fd))
			return -1;
	}
	return 0;

}

int sn_contrl_write(HSB_Object_Function *sn_function, void *read_buf)
{
	int i,index,cmd_cur;
	int len_config,len_diff,len_mod;
	int ret = -1;
	char sn_info[64] = {0};
	char key[32] = {0};
	char line_str[96] = {0};

	char config_diff[96] = {0};
	char uvc_config_buf[1024] ={0};
	char attr_string[128] = {0};

	FILE *config_fd = NULL;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	memcpy(sn_info, read_buf + HID_HEAD_SIZE, sn_function->hsb_size_len);
	cmd_cur = head->cmd >> 4;
	index = sizeof(hsb_string) / sizeof(struct uvc_hsb_string);
	for(i = 0; i < index ; i++){
		if(cmd_cur == hsb_string[i].id){
			strcpy(attr_string, hsb_string[i].s);
			break;
		}
		else if(i == index - 1){
			HID_WARNING("not No corresponding option\n");
			return -1;
		}

	}


	config_fd = fopen(sn_function->file_path, "rb+");
	if(config_fd == NULL){
		HID_ERR("open is uvc.config is failed\n");
		return -1;
	}
	fseek(config_fd, 0L, SEEK_END);
	len_config = ftell(config_fd);
	fseek(config_fd, 0L, SEEK_SET);
	/*
	 *HID_ERR("############### length = %d ################\n", ftell(config_fd));
	 *HID_ERR("uvc_hsb_string[i].s=%s\n",hsb_string[i].s);
	 */

	while (!feof(config_fd)) {
		if (fscanf(config_fd, "%[^\n]", line_str) < 0){
			HID_WARNING("fscanf is faild\n");
			break;
		}
		if (sscanf(line_str, "%[^:]", key) != 1) {
			HID_WARNING("warning: invalid param:%s\n", line_str);
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
				HID_ERR("fread is faild\n");

			fseek(config_fd, -(strlen(line_str) + len_diff), SEEK_CUR);
			fprintf(config_fd, "%s  :%s", attr_string, sn_info);
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
	cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
	fclose(config_fd);
	return 0;
}

int sn_burning_cmd(char *read_buf)
{
	int ret;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	HSB_Object_Function *sn_function;
	sn_function = malloc(sizeof(HSB_Object_Function));
	if(!sn_function){
		HID_ERR("sn_function is malloc failed\n");
		return -1;
	}
	sn_function->file_path = UPDATE_UVCCONFIG_PATH;

	switch(head->cmd){
	case HSB_WRITE_MANUFACTURE:
	case HSB_WRITE_SREIALNUMBER:
	case HSB_WRITE_PRODUCTNUMBER:
	case HSB_WRITE_PID:
	case HSB_WRITE_VID:
	case HSB_WRITE_BCD:
	case HSB_WRITE_MODEL:
	case HSB_WRITE_CMEI:
	case HSB_WRITE_APPKEY:
		sn_function->hsb_size_len = head->data_len;
		ret = sn_contrl_write(sn_function, head);
		if(ret < 0)
			cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		break;
	case HSB_READ_MANUFACTURE:
	case HSB_READ_SREIALNUMBER:
	case HSB_READ_PRODUCTNUMBER:
	case HSB_READ_PID:
	case HSB_READ_VID:
	case HSB_READ_BCD:
	case HSB_READ_MODEL:
	case HSB_READ_CMEI:
	case HSB_READ_APPKEY:
		ret = sn_contrl_read(sn_function, (void *)head);
		if(ret < 0)
			cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);

		break;
	}
	free(sn_function);
	return 0;
}
