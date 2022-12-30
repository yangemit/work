#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <linux/input.h>

#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_osd.h>

/* #include <multi_media_base.h> */
#include <imp-common.h>

/* 人脸个数通过修改宏：MAX_IVS_OSD_REGION */
extern IMPRgnHandle ivsRgnHandler[MAX_IVS_OSD_REGION];

enum _dir {
	DIR_FRONT = 0,
	DIR_BACK = DIR_FRONT,
	DIR_LEFT = 1,
	DIR_RIGHT = 2,
	DIR_LEFT_SKEWD = 3,
	DIR_RIGHT_SKEWD = 4,
	DIR_UB_FRONT = 5,
	DIR_FB_FRONT = 6,
};

static int drawLineRectShow(IMPRgnHandle handler, int x0, int y0, int x1, int y1)
{
	IMPOSDRgnAttr rAttr;
	if (IMP_OSD_GetRgnAttr(handler, &rAttr) < 0) {
		printf("%s(%d):IMP_OSD_GetRgnAttr failed\n", __func__, __LINE__);
		return -1;
	}
	rAttr.type = OSD_REG_RECT;
	rAttr.rect.p0.x = x0;
	rAttr.rect.p0.y = y0;
	rAttr.rect.p1.x = x1;
	rAttr.rect.p1.y = y1;
	/* rAttr.data.lineRectData.color = OSD_GREEN; */
	rAttr.data.lineRectData.color = OSD_RED;
	rAttr.data.lineRectData.linewidth = 4;

	/*if (IMP_OSD_SetRgnAttrWithTimestamp(handler, &rAttr, prTs) < 0) {*/
	if (IMP_OSD_SetRgnAttr(handler, &rAttr) < 0) {
		printf("%s(%d):IMP_OSD_SetRgnAttrWithTimestamp failed\n", __func__, __LINE__);
		return -1;
	}

	if (IMP_OSD_ShowRgn(handler, 0, 1) < 0) {
		printf("%s(%d): IMP_OSD_ShowRgn failed\n", __func__, __LINE__);
		return -1;
	}

	return 0;
}

void osd_render(int index, int x, int y, int w, int h)
{
	IMPRgnHandle handler;

	handler = ivsRgnHandler[index];
	drawLineRectShow(handler, x, y, x + w, y + h);
}

void osd_render_cls(void)
{
	IMPRgnHandle handler;
	int i = 0;
	for (i = 0; i < MAX_IVS_OSD_REGION; i++) {
		handler = ivsRgnHandler[i];
		IMP_OSD_ShowRgn(handler, 0, 0);
	}
}
