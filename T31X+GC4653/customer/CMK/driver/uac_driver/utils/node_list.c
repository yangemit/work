/**
 * node_list.c
 **/
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/completion.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <linux/list.h>

#include "node_list.h"

node_t *get_use_node(node_list_t *list)
{
	if (list) {
		unsigned long lock_flags;
		node_t *node = NULL;
		struct list_head *head = &list->use;

		if (!list->consumer_block && list_empty(head))
			return NULL;

		if (list->consumer_block)
			wait_for_completion(&list->comp_use);

		spin_lock_irqsave(&list->lock, lock_flags);
		{
			node = list_entry(head->next, node_t, head);
			list_del(head->next);
			list->use_cnt--;
		}
		spin_unlock_irqrestore(&list->lock, lock_flags);

		return node;
	}

	return NULL;
}

int put_free_node(node_list_t *list, node_t *node)
{
	if (list) {
		unsigned long lock_flags;
		struct list_head *head = &list->free;

		if (node == NULL)
			return -1;

		spin_lock_irqsave(&list->lock, lock_flags);
		{
			list_add_tail(&node->head, head);
			list->free_cnt++;
		}
		spin_unlock_irqrestore(&list->lock, lock_flags);

		if (list->producer_block)
			complete(&list->comp_free);

		return 0;
	}

	return -1;
}

node_t *get_free_node(node_list_t *list)
{
	if (list) {
		node_t *node = NULL;
		unsigned long lock_flags;
		struct list_head *free_head = &list->free;

		if (list_empty(free_head) && list->keep_update) {
			node_t *use_node = NULL;

			if (list->use_cnt >= (list->nodes - 1)) {
				use_node = get_use_node(list);;
				if (use_node)
					return use_node;
				else if (!list->producer_block && list_empty(free_head))
					return NULL;
			}
		}

		if (list->producer_block)
			wait_for_completion(&list->comp_free);

		spin_lock_irqsave(&list->lock, lock_flags);
		{
			node = list_entry(free_head->next, node_t, head);
			list_del(free_head->next);
			list->free_cnt--;
		}
		spin_unlock_irqrestore(&list->lock, lock_flags);

		return node;
	}

	return NULL;
}

int put_use_node(node_list_t *list, node_t *node)
{
	if (list) {
		unsigned long lock_flags;
		struct list_head *head = &list->use;

		if (node == NULL)
			return -1;

		spin_lock_irqsave(&list->lock, lock_flags);
		{
			list_add_tail(&node->head, head);
			list->use_cnt++;
		}
		spin_unlock_irqrestore(&list->lock, lock_flags);

		if (list->consumer_block)
			complete(&list->comp_use);

		return 0;
	}

	return -1;
}

int drop_use_node(node_list_t *list, node_t *node_ref)
{
	if (list) {
		node_t *node = NULL;
		unsigned long lock_flags;
		struct list_head *head = &list->use;
		struct list_head *pos;

		if (node_ref == NULL) {
			return -1;
		}

		spin_lock_irqsave(&list->lock, lock_flags);
		{
			if ((list->use_cnt <= 0) || (head->next == head)) {
				spin_unlock_irqrestore(&list->lock, lock_flags);
				return 0;
			}

			list_for_each (pos, head) {
				node = list_entry(pos, node_t, head);
				if (node == NULL) {
					return 0;
				}

				if (node->data == node_ref->data) {
					break;
				}
			}
			if (node && (pos != head)) {
				list_del(pos);
				list->use_cnt--;
			}
		}
		spin_unlock_irqrestore(&list->lock, lock_flags);

		return 0;
	}

	return -1;
}

node_list_t *init_node_list(int nodes, int size, int producer_block,
				int consumer_block, int keep_update)
{
	int i;
	void *info_buf = NULL;
	node_list_t *list = NULL;

	info_buf = kzalloc(sizeof(node_list_t) + nodes * sizeof(node_t), GFP_KERNEL);
	if (!info_buf) {
		printk("%s: ERROR: alloc memory error!\n", __func__);
		return NULL;
	}
	list = (node_list_t *)info_buf;
	list->data_buf = dma_alloc_coherent(NULL, nodes * size, &list->data_phy, GFP_KERNEL);

	/* printk("list->data_phy = 0x%x. list->data_buf = 0x%p\n", list->data_phy, list->data_buf); */
	if (!list->data_buf) {
		/* WARNING: here didn't release the resource have requested */
		printk("%s: ERROR: dma alloc coherent buf error!\n", __func__);
		return NULL;
	}

	memset(list->data_buf, 0x0, nodes * size);

	INIT_LIST_HEAD(&list->use);
	INIT_LIST_HEAD(&list->free);

	spin_lock_init(&list->lock);
	init_completion(&list->comp_use);
	init_completion(&list->comp_free);

	list->keep_update = keep_update;
	list->producer_block = producer_block;
	list->consumer_block = consumer_block;
	list->size = size;
	list->nodes = nodes;

	for (i = 0; i < nodes; i++) {
		node_t *node = (node_t *)((unsigned char *)info_buf + sizeof(node_list_t) + i * sizeof(node_t));
		node->size = 0;
		node->data = list->data_buf + i * size;
		node->data_phy = list->data_phy + i * size;
		node->crc32 = 0;
		node->private = 0;

		put_free_node(list, node);
	}

	return list;
}
