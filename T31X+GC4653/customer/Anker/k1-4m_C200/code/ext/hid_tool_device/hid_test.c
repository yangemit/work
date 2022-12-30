#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include "hid_test.h"

/*
 *#include <ivs/ivs_common.h>
 *#include <ivs/ivs_interface.h>
 *#include <ivs/ivs_inf_personDet.h>
 *#include <imp/imp_log.h>
 *#include <imp/imp_ivs.h>
 *#include <imp/imp_common.h>
 *#include <imp/imp_system.h>
 *#include <imp/imp_framesource.h>
 *
 *#include <iaac/iaac.h>
 */

hid_context *gcontext = NULL;
sem_t haa_flag;

/*
static IAACAuthInfo *hid_authinfo;
 *static int8_t haa_device_flag = 0;
 *static int last_cmd;
 *static int expect_cmd;
 *static int cur_cmd;
 *static int reset_cnt;
 */

/*sem_t haa_flag;*/
/*
crc生成
buf:
len:
crc:注意第一次调这个函数的时候crc初值要设成0xFFFF
return:
16bit的crc结果
*/
uint16_t soft_crc16(uint8_t *buf, int32_t len, uint16_t crc)
{
	/* HID_ERR("buf:%s,len:%d,crc:%d\n", buf, len, crc); */
	uint32_t i, flag;
	while(len-- > 0)
	{
		crc ^= *buf++;
		i = 8;
		while(i-- > 0)
		{
			flag = crc & 0x01;
			crc >>= 1;
			if(flag)
			{
				crc ^= 0x1021;//0x8005,0x1021
			}
		}
	}
	/* HID_ERR("crc16:%d\n", crc); */
	return crc;
}

int cmd_response(uint16_t cmd, uint32_t cmd_status, uint8_t data_status,
				 char *data_buf, uint16_t data_len)
{
	int w_len;
	/* int total_len = 0; */
	uint16_t resp_crc;

	response_cmd_head *resp = (response_cmd_head *)(gcontext->resp_buf);
	uint8_t *resp_buf = gcontext->resp_buf;

	resp->report_id = HID_REPORT_ID;
	resp->magic = MAGIC;
	resp->cmd = cmd;
	resp->cmd_status = cmd_status;

	if ((data_status == HID_DATA_VALID) && (data_len > 0)) {
		/* there is data for the cmd response */
		resp->data_status = HID_DATA_VALID;
		resp->data_len = data_len;
		memcpy(resp_buf + HID_RESPONSE_SIZE, data_buf, data_len);

		/* calc crc */
		resp_crc = soft_crc16(resp_buf, HID_RESPONSE_SIZE - sizeof(uint16_t) , 0xFFFF);
		resp->crc = soft_crc16(resp_buf + HID_RESPONSE_SIZE, data_len, resp_crc);

	} else {
		/* there is no data for the cmd response */
		resp->data_status = HID_DATA_INVALID;
		resp->data_len = 0;

		/* calc crc */
		resp->crc = soft_crc16(resp_buf, sizeof(response_cmd_head) - sizeof(uint16_t) , 0xFFFF);
	}
	/*for windows, the send len should be the report size */
	w_len = write(gcontext->hid_fd, resp_buf, BUF_LEN);
	if (w_len < 0) {
		HID_ERR("hid send response failed.\n");
		return -1;
	}

#if HID_DEBUG
	HID_LOG("###cmd end response:hid report_id=0X%X, magic=0X%X, cmd=0X%X, data_valid=0X%X, cmd_status=0X%X, crc=0X%X,data_len=%d\n",resp->report_id, resp->magic, resp->cmd,
		   resp->data_status, resp->cmd_status, resp->crc, data_len);

#endif

	return 0;
}

static int kiva_version_read(hid_cmd_head *head)
{
	FILE *config_fd = NULL;
	char line_str[24] = {0};

	config_fd = fopen(KIVA_VERSION_PATH, "r+");
	if(config_fd == NULL){
		HID_ERR("open is version.txt is failed\n");
		return -1;
	}

	fseek(config_fd, 0L, SEEK_SET);

	if (fscanf(config_fd, "%[^\n]", line_str) < 0)
		HID_WARNING("fscanf is faild\n");

	fclose(config_fd);

	if(strlen(line_str)){
		cmd_response(head->cmd, CMD_EXCUTE_OK, HID_DATA_VALID, line_str, strlen(line_str));
		return 0;
	}
	return -1;
}

static int common_usage_cmd(char *read_buf)
{
	int ret;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	switch(head->cmd){
		case HCU_GET_VERSION:
			ret = kiva_version_read(head);
			if(ret < 0)
				cmd_response(head->cmd, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);

			break;
	}
	return 0;
}

int hid_cmd_process(char *buf, int len)
{
	hid_cmd_head *head;
	uint16_t cmd_function;
	uint16_t cmd_crc, tmp_crc;

	head = (hid_cmd_head*)buf;
#if HID_DEBUG
	HID_LOG("###start:hid report_id=0X%X, magic=0X%X, cmd=0X%X, data_valid=0X%X, data_len=0X%X, crc=0X%X\n",head->report_id, head->magic, head->cmd,
			head->data_status, head->data_len, head->crc);
	 /*
	  *int i;
	  *for (i = 0; i < sizeof(hid_cmd_head)+head->data_len; i++) {
	  *       HID_ERR("0x%x,", buf[i]);
	  *       HID_ERR("\n");
	  *}
	  */
#endif

	if (head->report_id != HID_REPORT_ID) {
		HID_ERR("hid recv err report_id(%x).\n", head->report_id);
		cmd_response(head->cmd, CMD_REPORTID_ERROR, HID_DATA_INVALID, NULL, 0);
		return -1;
	}

	if (head->magic != MAGIC) {
		HID_ERR("hid recv err maigc(%x).\n", head->magic);
		cmd_response(head->cmd, CMD_MAGIC_ERROR, HID_DATA_INVALID, NULL, 0);
		return -1;
	}

	/* calc the crc */
	if ((head->data_status == HID_DATA_VALID ) ||  (head->data_status == HID_DATA_END)) {
		/* calc crc */
		tmp_crc = soft_crc16((uint8_t *)buf, HID_HEAD_SIZE - sizeof(uint16_t), 0xFFFF);
		cmd_crc = soft_crc16((uint8_t *)buf + HID_HEAD_SIZE, head->data_len, tmp_crc);
#if HID_DEBUG
		HID_LOG("%s:%d tmp_crc = %X\n", __func__, __LINE__, tmp_crc);
		HID_LOG("%s:%d cmd_crc = %X\n", __func__, __LINE__, cmd_crc);
#endif
	} else {
		cmd_crc = soft_crc16((uint8_t *)buf, HID_HEAD_SIZE - sizeof(uint16_t), 0xFFFF);
	}

	if (cmd_crc != head->crc) {
		HID_ERR("hid recv err head->crc = %x, cmd_crc = %x.\n", head->crc, cmd_crc);
		cmd_response(head->cmd, CMD_CRC_ERROR, HID_DATA_INVALID, NULL, 0);
		return -1;
	}

	cmd_function = head->cmd & 0xF000;
	switch (cmd_function) {
	case HID_COMMON_USAGE:
		common_usage_cmd(buf);
		break;
	case HID_ALGORITHM_AUTHORIZE:
		#ifdef HAA_ON
		algorithm_authorize_cmd(buf);
		#endif
		break;
	case HID_SN_BURNING:
		sn_burning_cmd(buf);
		break;
	case HID_ONLINE_UPDATE:
		process_update_cmd(buf, gcontext);
		break;
	default:
		cmd_response(head->cmd, CMD_UNSUPPORT_FUNCTION, HID_DATA_INVALID, NULL, 0);
		break;
	}
	return 0;
}

int ucamera_emer_config()
{
	int ret =0;
	int flag = -1;
	int ucamera_fd = -1;

	ucamera_fd = open(UCAMERA_DEVICE_PATH, O_RDWR, 0666);
	if (ucamera_fd < 0)
	{
		HID_ERR("open ucamera device faild\n");
		return -1;
	}
	ret = read(ucamera_fd, (char *)&flag, sizeof(int));
	if( ret < 0){
		HID_ERR("ucamera read is faild\n");
		return -1;
	}

	if(flag == 0) {
		printf("hid ioctl \n");
		ret = ioctl(ucamera_fd, UCAMERA_IOCTL_HID_EMER_ENABLE, NULL);
		if(ret < 0){
			HID_ERR("hid_test if ucamera ioctl if faild\n");
			return -1;
		}

		ret = read(ucamera_fd, (char *)&flag, 4);

		if( flag != 1){
			HID_ERR("emergency is failed flag=%d\n",flag);
			return -1;
		}

	}

	return 0;
}

void *hid_event_thread(void *arg)
{

	int fd = 0;
	/*int ret = -1;*/
	int cmd_len = 0;
	int rsize = 0;
	int rtotal = 0;
	fd_set rfds;
	int retval = 0;

	prctl(PR_SET_NAME, "hid_event");
	gcontext = malloc(sizeof(hid_context));
	if (!gcontext) {
		HID_ERR("malloc gcontext memory fail!\n");
		return NULL;
	}

	memset(gcontext, 0, sizeof(hid_context));
	gcontext->hid_fd = open(HID_DEVICE_PATH, O_RDWR, 0666);
	if (gcontext->hid_fd < 0) {
#if 0
		/*not set the devics to hid mode  */
		ret = ucamera_emer_config();
		if(!ret){
			gcontext->hid_fd = open(HID_DEVICE_PATH, O_RDWR, 0666);
			if (gcontext->hid_fd < 0){
				HID_ERR("hid_update open device faild!\n");
				return NULL;
			}
		}
#endif
		return NULL;
	}

	fd = gcontext->hid_fd;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	while (1) {
		retval = select(fd + 1, &rfds, NULL, NULL, NULL);
		if (retval == -1 && errno == EINTR)
			continue;
		if (retval < 0) {
			perror("select()");
			return NULL;
		}

		if (FD_ISSET(fd, &rfds)) {
			rtotal = 0;
			while (BUF_LEN - rtotal) {
				rsize = 0;
				rsize = read(fd, gcontext->read_buf + rtotal, BUF_LEN - rtotal);
				if (rsize < 0) {
					usleep(10 * 1000);
					HID_ERR("hid_cmd_process error.\n");
					break;
				}
				rtotal += rsize;
			}

			cmd_len = rtotal;
			hid_cmd_process(gcontext->read_buf, cmd_len);
		}
	}

	free(gcontext);
	close(fd);
	return NULL;
}

int main(void)
{
	int ret;

	sem_init(&haa_flag, 0, 0);
	pthread_t hid_pid;
	pthread_attr_t hid_attr;
	pthread_attr_init(&hid_attr);
	pthread_attr_setdetachstate(&hid_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&hid_attr, SCHED_OTHER);
	ret = pthread_create(&hid_pid, &hid_attr, &hid_event_thread, NULL);
	if (ret) {
		HID_ERR("create thread for osd_show failed!\n");
		return -1;
	}

	while (1) {
		sleep(60);
	}
	return 0;
}
