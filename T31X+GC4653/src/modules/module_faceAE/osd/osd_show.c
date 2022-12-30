#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <node_list.h>
#include <module_faceAE.h>

#define OSD_FACE    0
#define OSD_HUMAN   6

static int osd_stop = 0;
extern void osd_render(int index, int x, int y, int w, int h);
extern void osd_render_cls(void);

void *osd_show_thread(void *arg)
{
	int r_node;
	human_result_t *result = NULL;

	prctl(PR_SET_NAME, "osd_show");

	/* Initialize faceDet algorithm */

	while (!osd_stop) {
		/*Step1: get tesult from algorithm */
		r_node = module_algorithm_process_get_result(&result);
		if (!r_node) {

			osd_render_cls();
			usleep(100 * 1000);
			continue;
		}

		osd_render_cls();

		if (module_uvc_control_get_status()) {
			/*Step2: Draw OSD region to ch0 */
			for (int i = 0; i < result->human_cnt; i++) {
				osd_render(i, result->hobj[i].x, result->hobj[i].y, result->hobj[i].width, result->hobj[i].height);
				/* printf(" result->hobj[%d].x, y, width, height = %d,%d,%d,%d\n", i, result->hobj[i].x, result->hobj[i].y, result->hobj[i].width, result->hobj[i].height); */
			}
		}

		/* Give back algorithm node */
		module_algorithm_process_put_result(r_node);
	}

	return NULL;
}
