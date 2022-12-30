#ifndef __MODULE_TRACK_CONTROL_H
#define __MODULE_TRACK_CONTROL_H

#ifdef OSD_CONTROL
void module_track_put_result(int node);
int module_track_get_result(void **result);

typedef struct _human_object {
	int frame;
	int id;
	int x;
	int y;
	int width;
	int height;
} human_object_t;

typedef struct _human_result {
	int human_cnt;
	human_object_t hobj[8];
} human_result_t;
#endif

int module_track_init(void *param);
void module_track_deinit();

#endif
