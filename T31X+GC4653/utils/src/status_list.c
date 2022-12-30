#include <stdio.h>
#include <stdlib.h>
#include <status_list.h>
#include <string.h>

typedef struct _status_list_context {
	unsigned int node_num;
	unsigned int data_size;
	char *node_mem;
	status_list *status_list;
} status_list_ctx;

int init_status_list(unsigned int node_num, unsigned int node_data_size)
{
	if (node_num >= MAX_STATUS_LIST_NODE_NUM) {
		printf("ERROR(%s): node_num is invalid!\n", __func__);
		return 0;
	}

	if (node_data_size >= MAX_STATUS_LIST_NODE_DATA_SIZE) {
		printf("ERROR(%s): node_data_size is invalid!\n", __func__);
		return 0;
	}

	int i = 0;
	status_list_ctx *sl_ctx = (status_list_ctx *)malloc(sizeof(status_list_ctx) + node_num * sizeof(status_list) + node_num * node_data_size);
	if (!sl_ctx) {
		printf("ERROR(%s): malloc sl_ctx failed!\n", __func__);
		return 0;
	}
	memset(sl_ctx, 0, sizeof(status_list_ctx) + node_num * sizeof(status_list) + node_num * node_data_size);
	sl_ctx->node_num = node_num;
	sl_ctx->data_size = node_data_size;
	sl_ctx->status_list = (status_list *)((char *)sl_ctx + sizeof(status_list_ctx));

	if (node_data_size) {
		sl_ctx->node_mem = (char *)sl_ctx + sizeof(status_list_ctx) + node_num * sizeof(status_list);
		status_list *tmp = NULL;
		for (i = 0; i < node_num; i++) {
			tmp = (status_list *)((char *)sl_ctx->status_list + i * sizeof(status_list));
			tmp->data = sl_ctx->node_mem + i * node_data_size;
		}
	}

	return (int)sl_ctx;
}

void deinit_status_list(int context)
{
	free((void *)context);
}

int search_obj_in_status_list(int context, int id)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	int i = 0;
	status_list *tmp = NULL;
	for (i = 0; i < sl_ctx->node_num; i++) {
		tmp = (status_list *)((char *)sl_ctx->status_list + i * sizeof(status_list));
		if (tmp->valid) {
			if (tmp->id == id)
				break;
		}
	}

	return (i == sl_ctx->node_num ? -1 : i);
}

int insert_obj_to_status_list(int context, status_list *sl)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	if (!sl) {
		printf("ERROR(%s): sl is NULL!\n", __func__);
		return -1;
	}

	if (sl->data_size > MAX_STATUS_LIST_NODE_DATA_SIZE) {
		printf("ERROR(%s): data_size is invalid!\n", __func__);
		return -1;
	}

	int i = 0;
	status_list *tmp = NULL;
	for (i = 0; i < sl_ctx->node_num; i++) {
		tmp = (status_list *)((char *)sl_ctx->status_list + i * sizeof(status_list));
		if (tmp->valid == 0) {
			tmp->valid = 1;
			tmp->id = sl->id;
			tmp->ts = sl->ts;
			tmp->status = sl->status;
			tmp->data_size = sl->data_size;
			if (sl->data && sl->data_size > 0) {
				memcpy(tmp->data, sl->data, sl->data_size);
			}
			break;
		}
	}

	if (i == sl_ctx->node_num)
		printf("WARNING(%s): track table list full!\n", __func__);

	return (i == sl_ctx->node_num ? -1 : i);
}

int delete_oldest_obj_from_status_list(int context, unsigned long long ts, unsigned long long interval)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	int i = 0;
	status_list *tmp = NULL;
	for (i = 0; i < sl_ctx->node_num; i++) {
		tmp = (status_list *)((char *)sl_ctx->status_list + i * sizeof(status_list));
		if (tmp->valid == 1) {
			if ((ts - tmp->ts) >= interval) {
				tmp->valid = 0;
				tmp->id = 0;
				tmp->status = 0;
				tmp->ts = 0;
				tmp->data_size = 0;
				memset(tmp->data, 0, sl_ctx->data_size);
			}
		}
	}

	return 0;
}

int delete_obj_from_status_list_by_offset(int context, unsigned int offset)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	if (offset >= sl_ctx->node_num) {
		printf("ERROR(%s): offset is invalid!\n", __func__);
		return -1;
	}

	status_list *tmp = NULL;
	tmp = (status_list *)((char *)sl_ctx->status_list + offset * sizeof(status_list));

	tmp->valid = 0;
	tmp->id = 0;
	tmp->status = 0;
	tmp->ts = 0;
	tmp->data_size = 0;
	memset(tmp->data, 0, sl_ctx->data_size);

	return 0;
}

int get_obj_from_status_list(int context, unsigned int offset, status_list *sl)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	if (offset >= sl_ctx->node_num) {
		printf("ERROR(%s): offset is invalid!\n", __func__);
		return -1;
	}

	if (!sl) {
		printf("ERROR(%s): sl is invalid!\n", __func__);
		return -1;
	}

	status_list *tmp = NULL;
	tmp = (status_list *)((char *)sl_ctx->status_list + offset * sizeof(status_list));

	memset(sl, 0, sizeof(status_list));
	memcpy(sl, tmp, sizeof(status_list));

	return 0;
}

int update_obj_to_status_list(int context, unsigned int offset, status_list *sl)
{
	status_list_ctx *sl_ctx = (status_list_ctx *)context;
	if (!sl_ctx) {
		printf("ERROR(%s): context is invalid!\n", __func__);
		return -1;
	}

	if (offset >= sl_ctx->node_num) {
		printf("ERROR(%s): offset is invalid!\n", __func__);
		return -1;
	}

	status_list *tmp = NULL;
	tmp = (status_list *)((char *)sl_ctx->status_list + offset * sizeof(status_list));

	tmp->id = sl->id;
	tmp->status = sl->status;
	tmp->ts = sl->ts;
	tmp->data_size = sl->data_size;
	if (sl->data && sl->data_size > 0) {
		memset(tmp->data, 0, sl_ctx->data_size);
		memcpy(tmp->data, sl->data, sl->data_size);
	}

	return 0;
}
