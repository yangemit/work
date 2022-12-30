#ifndef __STATUS_LIST_H__
#define __STATUS_LIST_H__

#define MAX_STATUS_LIST_NODE_NUM        128
#define MAX_STATUS_LIST_NODE_DATA_SIZE  (1 * 1024 * 1024)

typedef struct _status_list {
	int valid;
	int id;
	int status;
	unsigned long long ts;
	unsigned int data_size;
	void *data;
} status_list;

/* init status list */
int init_status_list(unsigned int node_num, unsigned int node_data_size);

/* deinit status list */
void deinit_status_list(int context);

/* serach obj offset by id */
int search_obj_in_status_list(int context, int id);

/* insert new obj to status list */
int insert_obj_to_status_list(int context, status_list *sl);

/* get obj from status list by offset */
int get_obj_from_status_list(int context, unsigned int offset, status_list *sl);

/* update obj from status list by offset */
int update_obj_to_status_list(int context, unsigned int offset, status_list *sl);

/* delete obj from status list by offset */
int delete_obj_from_status_list_by_offset(int context, unsigned int offset);

/* clean status list by timestamp */
int delete_oldest_obj_from_status_list(int context, unsigned long long ts, unsigned long long interval);

#endif /*__STATUS_LIST_H__*/
