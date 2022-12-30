#ifndef __module_faceAE_H_
#define __module_faceAE_H_

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
int module_faceAE_init();
int module_algorithm_process_get_result(human_result_t **result);
void module_algorithm_process_put_result(int node);
#endif /* __module_faceAE_H_*/
