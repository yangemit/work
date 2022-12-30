/*********************************************************
 * File Name   : hou.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2020-10-15 10:53
 ********************************************************/
#include<stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "hid_test.h"

char sensor_config[128] = {0};
char file_path[128] = {0};
static int hou_flag = 1;


int hid_update_start(hid_context *gcontext)
{
	HID_LOG("hid update start.\n");
	if (!gcontext->hou_object.recv_buffer) {
		gcontext->hou_object.recv_buffer = malloc(UPDATE_BUFFER_MAX_SIZE);
		if (gcontext->hou_object.recv_buffer == NULL) {
			HID_ERR("%s:%d malloc recv buffer failed!\n",__func__, __LINE__);
			return -1;
		}
	}

	gcontext->hou_object.buffer_size = UPDATE_BUFFER_MAX_SIZE;
	gcontext->hou_object.recv_off = 0;
	gcontext->hou_object.last_pack_num = 0;
	gcontext->hou_object.file_path = NULL;
	gcontext->hou_object.data_trans_complete = 0;
	return 0;
}

int hid_update_send_data(char *read_buf, hid_context *gcontext)
{
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	if (head->pack_num > 1) {
		if ((gcontext->hou_object.last_pack_num + 1) !=  head->pack_num ) {
			HID_ERR("the pack num is not right, lost packet.\n");
			return -1;
		}
	}

	if ((gcontext->hou_object.recv_off + head->data_len) > gcontext->hou_object.buffer_size) {
		HID_WARNING("data transfer over the limit!\n");
		return -1;
	}

	memcpy(gcontext->hou_object.recv_buffer + gcontext->hou_object.recv_off, read_buf + HID_HEAD_SIZE, head->data_len);
	if (head->data_status == HID_DATA_END) {
		/* data transfer complete flag is setted */
		gcontext->hou_object.data_trans_complete = 1;
		hou_flag = 4;
	}

	gcontext->hou_object.recv_off +=  head->data_len;
	gcontext->hou_object.last_pack_num = head->pack_num;

	return 0;
}

int hid_update_get_result(hid_context *gcontext)
{
	char command[256] = {0};
	int w_len;

	FILE *fw_fd = NULL;

	sprintf(command, "rm %s", gcontext->hou_object.file_path);

	if (gcontext->hou_object.data_trans_complete == 1 ) {
		system(command);
		system("sync");
		fw_fd = fopen(gcontext->hou_object.file_path, "wb");
		if (fw_fd == NULL) {
			HID_ERR("update creat file faild.\n");
			return -1;
		}
		w_len = fwrite(gcontext->hou_object.recv_buffer, 1, gcontext->hou_object.recv_off, fw_fd);
		if (w_len != gcontext->hou_object.recv_off) {
			HID_ERR("update write data failed. \n");
			return -1;
		}
		HID_LOG("_______________wlen_____________ =%d\n",w_len);
		/* app and sensor bin need to be excuted */
		sprintf(command, "chmod 0777 %s", gcontext->hou_object.file_path);
		system(command);
		fclose(fw_fd);
	} else {
		HID_ERR("data transfer not complet!\n");
		return -1;
	}

	if(0 == strcmp(gcontext->hou_object.file_path, UPDATE_HID_TEST_PATH)){
		memset(command, 0, sizeof(command));
		sprintf(command, "cp %s %s", UPDATE_HID_TEST_PATH, UPDATE_HID_TEST_BAK_PATH);
		system("command");
		system("sync");
	}

	/* this time transfer complete, need release buffer */
	free(gcontext->hou_object.recv_buffer);
	gcontext->hou_object.recv_buffer = NULL;
	return 0;
}

int sensor_load_config(char *sensor_config, int flag)
{
	char key[64] = {0};
	char value[64] = {0};
	char *line_str = NULL;
	FILE *fp = NULL;
	memset(sensor_config, 0, sizeof(sensor_config));

	if ((fp = fopen(UPDATE_UVCCONFIG_PATH, "r")) == NULL) {
		HID_ERR("%s open config file failed!\n", __func__);
		return -1;
	}
	line_str = malloc(256*sizeof(char));

	/* printf("\n**********Config Param**********\n"); */
	while (!feof(fp)) {
		if (fscanf(fp, "%[^\n]", line_str) < 0)
			break;
		fseek(fp , 1, SEEK_CUR);

		if (sscanf(line_str, "%[^:]:%[^\n]", key, value) != 2) {
			HID_LOG("warning: skip config %s\n", line_str);
			fseek(fp , 1, SEEK_CUR);
			continue;
		}

		char *ch = strchr(key, ' ');
		if (ch) *ch = 0;
		if (strcmp(key, "sensor_name") == 0) {
			 printf("%s %s\n", key, value);
			if(flag == 1)
				sprintf(sensor_config, UPDATE_SENSOR_DRIVER_PATH, value);
				/*sprintf(strl, UPDATE_SENSOR_DRIVER_PATH, value);*/
			else if (flag == 2)
				sprintf(sensor_config, UPDATE_SENSOR_SETTINT_PATH, value);
			/*file_path = strl;*/
			HID_LOG("sensor_config =%s\n",sensor_config);
			fclose(fp);
			return 0;
		}

	}
	fclose(fp);
	return -1;

}

int process_update_cmd(char *read_buf, hid_context *gcontext)
{
	int ret = -1;
	int flag = 0;
	char *ch = NULL;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	switch(head->cmd){
	case HOU_START:
		if(hou_flag != 1){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		ret = hid_update_start(gcontext);
		if(ret < 0)
			ret = cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		else{
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			hou_flag = 2;
		}
		break;
	case HOU_UPDATE_UVCCONFIG:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		hou_flag = 3;
		gcontext->hou_object.file_path = UPDATE_UVCCONFIG_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_UVCATTR:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		hou_flag = 3;
		gcontext->hou_object.file_path = UPDATE_UVCATTR_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_UVCAPP:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		hou_flag = 3;
		gcontext->hou_object.file_path = UPDATE_UVCAPP_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_UVCAPP_ANTICOPY:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}
		hou_flag = 3;

		gcontext->hou_object.file_path = UPDATE_UVCAPP_ANTICOPY_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_APP_INIT_SH:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}
		hou_flag = 3;

		gcontext->hou_object.file_path = UPDATE_APP_INIT_SH_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_FAST_INIT_SH:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}
		hou_flag = 3;

		gcontext->hou_object.file_path = UPDATE_FAST_INIT_SH_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_HID_TEST:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}
		hou_flag = 3;

		gcontext->hou_object.file_path = UPDATE_HID_TEST_PATH;
		ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		break;
	case HOU_UPDATE_SENSOR_DIRVER:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		flag = 1;
		ret = sensor_load_config(sensor_config, flag);
		if(ret < 0)
			cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		else{
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			gcontext->hou_object.file_path = sensor_config;
			hou_flag = 3;
		}
		break;
	case HOU_UPDATE_SENSOR_SETTING:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		flag = 2;
		ret = sensor_load_config(sensor_config, flag);
		if(ret < 0)
			ret = cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		else{
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			gcontext->hou_object.file_path = sensor_config;
			hou_flag = 3;
		}
		break;
         case HOU_UPDATE_CUSTOMISE:
		if(hou_flag != 2){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}
		 hou_flag = 3;

		 memset(file_path, 0, sizeof(file_path));

		 memcpy(file_path, (char *)read_buf + HID_HEAD_SIZE, head->data_len);
		 gcontext->hou_object.file_path = file_path;
		 ch = strchr(gcontext->hou_object.file_path, ' ');
		 if(ch) *ch = 0;

		 printf("++++++++hou_object.file_path=%s+++++++++\n",gcontext->hou_object.file_path);

                 cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
                 break;
	case HOU_SEND_DATA:
		if(hou_flag != 3){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		ret = hid_update_send_data(read_buf, gcontext);
		if(ret < 0)
			ret = cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		else{
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
		}
		break;
	case HOU_GET_RESULT:
		if(hou_flag != 4){
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			break ;
		}

		ret = hid_update_get_result(gcontext);
		if (ret < 0)
			ret = cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		else{
			ret = cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
			hou_flag = 1;
		}
		break;
	default:
		break;
	}

	return ret;
}
