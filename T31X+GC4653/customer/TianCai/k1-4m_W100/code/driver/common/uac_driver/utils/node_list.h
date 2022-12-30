#ifndef __NODE_LIST_H__
#define __NODE_LIST_H__

#define NL_BLOCK          1
#define NL_NO_BLOCK       0

#define NL_KEEPUPDATE     1
#define NL_NO_KEEPUPDATE  0

typedef struct _node_t {
	struct list_head head;
	unsigned int size;		/* reserved for future use */
	void *data;			/* need alloc memory */
	unsigned int data_phy;		/* physical address of data */
	unsigned int crc32;			/* crc32 */
	int private;                    /* private */
	int width;
	int height;
	int video_size;
	char ts[16];
} node_t;

typedef struct node_list {
	unsigned int free_cnt;
	struct list_head free;	/* free list */
	unsigned int use_cnt;
	struct list_head use;	/* use list */
	struct completion comp_use;	/* use for blockd read */
	struct completion comp_free;	/* use for blockd read */
	spinlock_t lock;
	void *data_buf;			/* data head */
	unsigned int data_phy;	/* physical address of data head */
	int keep_update;        /* keep update */
	int producer_block;
	int consumer_block;
	int size;               /* node memory size */
	unsigned int nodes;     /* total node */
} node_list_t;

node_t *get_use_node(node_list_t *list);
int put_free_node(node_list_t *list, node_t *node);
node_t *get_free_node(node_list_t *list);
int put_use_node(node_list_t *list, node_t *node);
int drop_use_node(node_list_t *list, node_t *node_ref);
node_list_t *init_node_list(int nodes, int size, int producer_block,
				int consumer_block, int keep_update);

#endif //__NODE_LIST_H__
