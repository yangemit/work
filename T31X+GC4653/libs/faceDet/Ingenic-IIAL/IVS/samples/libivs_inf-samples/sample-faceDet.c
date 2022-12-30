#include <string.h>
#include <unistd.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_system.h>
#include <imp/imp_framesource.h>

#include <ivs/ivs_common.h>
#include <ivs/ivs_interface.h>
#include <ivs/ivs_inf_faceDet.h>
#include <imp/imp_ivs.h>

#include <iaac.h>

#include "sample-common.h"

#define TAG "SAMPLE-FACEDET"
#define TIME_OUT 1500

int sample_ivs_facedet_init(int grp_num) {
    int ret = 0;

    ret = IMP_IVS_CreateGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    IAACInfo ainfo = {
        .license_path = "./license.txt",
        .cid = 1,
        .fid = 1,
        .sn = "ae7117082a18d846e4ea433bcf7e3d2b",
    };

    ret = IAAC_Init(&ainfo);
    if (ret) {
        IMP_LOG_ERR(TAG, "IAAC_Init error!\n");
        return -1;
    }

    return 0;
}

int sample_ivs_facedet_exit(int grp_num) {
    int ret = 0;

    IAAC_DeInit();
    
    ret = IMP_IVS_DestroyGroup(grp_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
        return -1;
    }
    return 0;
}

int sample_ivs_facedet_start(int grp_num, int chn_num, IMPIVSInterface **interface) {
    int ret = 0;
    facedet_param_input_t param;

    memset(&param, 0, sizeof(facedet_param_input_t));
    param.frameInfo.width = sensor_sub_width;
    param.frameInfo.height = sensor_sub_height;

    param.skip_num = 0;
    param.max_face_box = 10;
    param.sense = 1;         //detection sensibility
    param.detdist = 0;         //detection distance
    param.rot90 = false; // true:图像顺时针旋转90度 false:图像不旋转
    param.switch_liveness = false; // true: 做人脸活体检测; false: 不做人脸活体检测
    param.switch_face_pose = true; // true: 做人脸角度检测; false: 不做人脸角度检测
    param.switch_face_blur = true; // true: 做人脸模糊检测; false: 不做人脸模糊检测

    *interface = FaceDetInterfaceInit(&param);
    if (*interface == NULL) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
        return -1;
    }

    ret = IMP_IVS_CreateChn(chn_num, *interface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
        return -1;
    }

    ret = IMP_IVS_RegisterChn(grp_num, chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
        return -1;
    }

    ret = IMP_IVS_StartRecvPic(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
        return -1;
    }

    return 0;
}

int sample_ivs_facedet_stop(int chn_num, IMPIVSInterface *interface) {
    int ret = 0;

    ret = IMP_IVS_StopRecvPic(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
        return -1;
    }
    sleep(1);

    ret = IMP_IVS_UnRegisterChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
        return -1;
    }

    ret = IMP_IVS_DestroyChn(chn_num);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
        return -1;
    }

    FaceDetInterfaceExit(interface);

    return 0;
}


int main(int argc, char *argv[]) {
    printf("face detect\n");
    int ret = 0, i = 0;
    IMPIVSInterface *inteface = NULL;
    facedet_param_output_t *result = NULL;

	/* Step.1 System init */
    ret = sample_system_init();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
        return -1;
	}

    /* Step.2 FrameSource init */
    IMPFSChnAttr fs_chn_attr;
    memset(&fs_chn_attr, 0, sizeof(IMPFSChnAttr));
    fs_chn_attr.pixFmt = PIX_FMT_NV12;
    /* fs_chn_attr.pixFmt = PIX_FMT_BGR24; */
    fs_chn_attr.outFrmRateNum = SENSOR_FRAME_RATE;
    fs_chn_attr.outFrmRateDen = 1;
    /* fs_chn_attr.nrVBs = 2; */
    fs_chn_attr.nrVBs = 3;
    fs_chn_attr.type = FS_PHY_CHANNEL;

    fs_chn_attr.crop.enable = 0;
    fs_chn_attr.crop.top = 0;
    fs_chn_attr.crop.left = 0;
    fs_chn_attr.crop.width = sensor_main_width;
    fs_chn_attr.crop.height = sensor_main_height;

    fs_chn_attr.scaler.enable = 1;	/* ivs use the second framesource channel, need scale*/
    fs_chn_attr.scaler.outwidth = sensor_sub_width;
    fs_chn_attr.scaler.outheight = sensor_sub_height;

    fs_chn_attr.picWidth = sensor_sub_width;
    fs_chn_attr.picHeight = sensor_sub_height;

    ret = sample_framesource_init(FS_SUB_CHN, &fs_chn_attr);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource init failed\n");
        return -1;
    }

    /* Step.3 Encoder init */
    ret = sample_ivs_facedet_init(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_facedet_init(0) failed\n");
        return -1;
    }

    /* Step.4 Bind */
    IMPCell framesource_cell = {DEV_ID_FS, FS_SUB_CHN, 0};
    IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};

    ret = IMP_System_Bind(&framesource_cell, &ivs_cell);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.5 Stream On */
    IMP_FrameSource_SetFrameDepth(FS_SUB_CHN, 0);
    ret = sample_framesource_streamon(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
        return -1;
    }

    ret = sample_ivs_facedet_start(0, 0, &inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_facedet_start(0, 0) failed\n");
        return -1;
    }

    /* Step.6 Get result */
    /* for (i = 0; i < NR_FRAMES_TO_IVS; i++) { */
    for (i = 0; i < 10000; i++) {
        ret = IMP_IVS_PollingResult(0, TIME_OUT);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", 0, IMP_IVS_DEFAULT_TIMEOUTMS);
            return -1;
        }
        ret = IMP_IVS_GetResult(0, (void **)&result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", 0);
            return -1;
        }
        facedet_param_output_t* r = (facedet_param_output_t*)result;
        int i = 0;
        for(i = 0; i < r->count; i++) {
            IVSRect* rect = &r->face[i].box;
            printf("face location:%d, %d, %d, %d\n", rect->ul.x, rect->ul.y, rect->br.x, rect->br.y);
            printf("face pose:%f, %f, %f\n", r->face[i].face_pose_res.yaw, r->face[i].face_pose_res.pitch, r->face[i].face_pose_res.roll);
            printf("face confidence:%f\n", r->face[i].confidence);
        }

        ret = IMP_IVS_ReleaseResult(0, (void *)result);
        if (ret < 0) {
            IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", 0);
            return -1;
        }
#if 0
        if (i % 20 == 0) {
            ret = sample_ivs_set_sense(0, i % 5);
            if (ret < 0) {
                IMP_LOG_ERR(TAG, "sample_ivs_set_sense(0, %d) failed\n", i % 5);
                return -1;
            }
        }
#endif
    }

    ret = sample_ivs_facedet_stop(0, inteface);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_ivs_facedet_stop(0) failed\n");
        return -1;
    }

    /* Step.7 Stream Off */
    ret = sample_framesource_streamoff(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
        return -1;
    }

    /* Step.b UnBind */
    ret = IMP_System_UnBind(&framesource_cell, &ivs_cell);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.c ivs exit */
    ret = sample_ivs_facedet_exit(0);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "ivs mode exit failed\n");
        return -1;
    }

    /* Step.d FrameSource exit */
    ret = sample_framesource_exit(FS_SUB_CHN);
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "FrameSource(%d) exit failed\n", FS_SUB_CHN);
        return -1;
    }

    /* Step.e System exit */
    ret = sample_system_exit();
    if (ret < 0) {
        IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
        return -1;
    }

    return 0;
}
