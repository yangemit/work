/*********************************************************
 * File Name   : haa.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2020-10-15 10:53
 ********************************************************/
#include <stdio.h>
#include <pthread.h>
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

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_personDet.h>
#include <imp/imp_log.h>
#include <imp/imp_ivs.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <iaac/iaac.h>
#include "hid_test.h"

static IAACAuthInfo *hid_authinfo;
static int last_cmd;
static int expect_cmd;
static int cur_cmd;
static int reset_cnt;

extern sem_t haa_flag;

int hid_send_recv_data(IAACAuthInfo *authInfo)
{
	hid_authinfo = authInfo;
	sem_wait(&haa_flag);
	HID_LOG("sem_wait is stratct\n");
	if(authInfo->need_send_data_len){
		cmd_response(HAA_DEVICE_INFO, CMD_EXCUTE_OK, HID_DATA_VALID, (char *)authInfo, sizeof(IAACAuthInfo));
	} else {
		HID_ERR("need send data is faild \n");
		cmd_response(HAA_DEVICE_INFO, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		return -1;

	}
	memset(authInfo->need_recv_data, 0, authInfo->need_recv_data_len);
	sem_wait(&haa_flag);
	return 0;
}

int sample_ivs_persondet_init(int grp_num ,struct algorithm_info *host_algorithm_info)
{
    int ret = 0;

    ret = IMP_IVS_CreateGroup(grp_num);
    if (ret < 0) {
        HID_ERR("imp_ivs_creategroup() failed\n");
        return -1;
    }

   static IAACInfo ainfo = {
	    .license_path = HAA_LICENSE_PATH,
	    .cid = 1,
	    .fid = 1,
	    .sn = "12345",
	    .send_and_recv = hid_send_recv_data,
    };

    ainfo.cid = host_algorithm_info->cid;
    ainfo.fid = host_algorithm_info->fid;
    ainfo.sn = host_algorithm_info->sn;

    ret = IAAC_Init(&ainfo);
	if (ret) {
		HID_ERR("IAAC_Init error!\n");
		cmd_response(HAA_START, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
		return -1;
	}
	cmd_response(HAA_START, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);

    return 0;
}

int sample_ivs_persondet_exit(int grp_num) {
    int ret = 0;

    IAAC_DeInit();

    ret = IMP_IVS_DestroyGroup(grp_num);
    if (ret < 0) {
        HID_ERR("IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
        return -1;
    }
    return 0;
}

int sample_ivs_persondet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
    int ret = 0;

    persondet_param_input_t param;

    memset(&param, 0, sizeof(persondet_param_input_t));
    param.frameInfo.width = 800; //800;
    param.frameInfo.height = 800; //800;

    param.skip_num = 3;      //skip num
    param.ptime = false;     //print time or not
    param.sense = 5;         //detection sensibility
    param.detdist = 1;         //detection distance

    *interface = PersonDetInterfaceInit(&param);
    if (*interface == NULL) {
        HID_ERR("IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    ret = IMP_IVS_CreateChn(chn_num, *interface);
    if (ret < 0) {
        HID_ERR("IMP_IVS_CreateChn(%d) failed\n", chn_num);
	cmd_response(HAA_WARRANT_INFO, CMD_EXCUTE_ERROR, HID_DATA_INVALID, NULL, 0);
        return -1;
    }
    cmd_response(HAA_WARRANT_INFO, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
    HID_LOG("+++++++++++++++ lgorithm authorization is success+++++++++++++++++++\n");

    ret = IMP_IVS_RegisterChn(grp_num, chn_num);
    if (ret < 0) {
        HID_ERR("IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
        return -1;
    }

    return 0;
}

int sample_ivs_persondet_stop(int chn_num, IMPIVSInterface *interface) {
    int ret = 0;


    ret = IMP_IVS_UnRegisterChn(chn_num);
    if (ret < 0) {
        HID_ERR("IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
        return -1;
    }

    ret = IMP_IVS_DestroyChn(chn_num);
    if (ret < 0) {
        HID_ERR("IMP_IVS_DestroyChn(%d) failed\n", chn_num);
        return -1;
    }

    PersonDetInterfaceExit(interface);

    return 0;
}
void *algorithm_authorize_start(void *read_buf)
{
	int ret;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;

	struct algorithm_info *host_algorithm_info;
	host_algorithm_info = malloc(sizeof(struct algorithm_info));
	if (!host_algorithm_info) {
		HID_ERR("malloc host_algorithm_info memory fail!\n");
		return NULL;
	}
	memcpy(host_algorithm_info, read_buf + HID_HEAD_SIZE, head->data_len);

	IMPIVSInterface *inteface = NULL;

	ret = IMP_System_Init();
	if(ret < 0){
		HID_ERR("IMP_System_Init failed\n");
		return NULL;
	}

	ret = sample_ivs_persondet_init(0, host_algorithm_info);
	if (ret < 0) {
		HID_ERR("sample_ivs_persondet_init(0) failed\n");
		free(host_algorithm_info);
		return NULL;
	}

	ret = sample_ivs_persondet_start(0, 0, &inteface);
	if (ret < 0) {
		HID_ERR("sample_ivs_persondet_start(0, 0) failed\n");
		free(host_algorithm_info);
		return NULL;
	}

	free(host_algorithm_info);
	ret = sample_ivs_persondet_stop(0, inteface);
	if (ret < 0) {
		HID_ERR("sample_ivs_persondet_stop(0) failed\n");
		return NULL;
	}


	IMP_System_Exit();

	return NULL;

}

int algorithm_authorize_cmd(char *read_buf)
{
	int ret;
	hid_cmd_head *head = (hid_cmd_head*)read_buf;
	pthread_t haa_algor_author_start;
	switch(head->cmd){
	case HAA_START:
		cur_cmd = HAA_START;
		if (expect_cmd) {
			if (cur_cmd != expect_cmd) {
				HID_WARNING("(%s): cur_cmd(0x%04x) is not expect_cmd(0x%04x)!\n", __func__, cur_cmd, expect_cmd);
				reset_cnt++;
				if (reset_cnt >= 5) {
					reset_cnt = 0;
					expect_cmd = 0;
				}
				cmd_response(HAA_START, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
				break;
			}
		}
		reset_cnt = 0;
		system("killall -USR2 ucamera");
		sleep(2);
		ret = pthread_create(&haa_algor_author_start, NULL, algorithm_authorize_start, read_buf);
		if(ret != 0)
			HID_ERR("pthread create haa_algor_author_start is failed\n");
		last_cmd = HAA_START;
		expect_cmd = HAA_DEVICE_INFO;
		break;
	case HAA_DEVICE_INFO:
		cur_cmd = HAA_DEVICE_INFO;
		if (expect_cmd) {
			if (cur_cmd != expect_cmd) {
			HID_WARNING("(%s): cur_cmd(0x%04x) is not expect_cmd(0x%04x)!\n", __func__, cur_cmd, expect_cmd);
				cmd_response(HAA_DEVICE_INFO, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
				break;
			}
		}
		sem_post(&haa_flag);
		last_cmd = HAA_DEVICE_INFO;
		expect_cmd = HAA_WARRANT_INFO;
		break;
	case HAA_WARRANT_INFO:
		cur_cmd = HAA_WARRANT_INFO;
		if (expect_cmd) {
			if (cur_cmd != expect_cmd) {
				HID_WARNING("WARNING(%s): cur_cmdTTT(0x%04x) is not expect_cmd(0x%04x)!\n", __func__, cur_cmd, expect_cmd);
				cmd_response(HAA_WARRANT_INFO, CMD_EXCUTE_OK, HID_DATA_INVALID, NULL, 0);
				break;
			}
		}
		memcpy(hid_authinfo->need_recv_data, read_buf + HID_HEAD_SIZE, head->data_len);
		hid_authinfo->need_recv_data_len = head->data_len;
		sem_post(&haa_flag);
		last_cmd = HAA_WARRANT_INFO;
		expect_cmd = 0;
		break;
	default:
		cmd_response(head->cmd, CMD_UNSUPPORT_FUNCTION, HID_DATA_INVALID, NULL, 0);
		break;
	}
	return 0;
}
