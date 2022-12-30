/*
 * cdc_log_info.c
 *
 * Copyright (C) 2021 Ingenic Semiconductor Co.,Ltd
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <sys/time.h>

#include <usbcamera.h>
#include <cdc_log_info.h>

static FILE *log_fp = NULL;
static hwvreg_write_data hwvreg_data;

void cdc_log_fun(int err_code, int line, const char*func, const char *fmt, ...)
{
	char msg_buf[20 * 1024];
	va_list ap;
	va_start(ap,fmt);
	sprintf(msg_buf, "FUNC(%s), LINE(%d), ERR_CODE(0x%x): ", func, line, err_code);
	vsprintf(msg_buf + strlen(msg_buf), fmt, ap);
	if (log_fp) {
		fprintf(log_fp, "%s\n", msg_buf);
		fflush(log_fp);
	} else
		fprintf(stderr, "%s\n", msg_buf);

	va_end(ap);
}

static int cdc_log_open()
{
	if (log_fp) {
		fclose(log_fp);
		log_fp = NULL;
	}
	log_fp = fopen(CDC_LOG_FILE, "w+");
	if (!log_fp) {
		printf("(%s:%d): open cdc_log_file error \n", __func__, __LINE__);
		return -1;
	}

	setbuf(log_fp,NULL);

	return 0;
}

static int get_log_size()
{
	struct stat statbuf;
	stat(CDC_LOG_FILE, &statbuf);

	int size = statbuf.st_size;

	return size;
}

int cdc_log_write()
{
	int ret = -1, size = 0;

	/*read data from log_file */
	if (log_fp) {
		fclose(log_fp);
		log_fp = NULL;
	}

	log_fp = fopen(CDC_LOG_FILE, "r+");
	if (!log_fp) {
		printf("(%s:%d): open cdc_log_file error \n", __func__, __LINE__);
		return -1;
	}

	memset(hwvreg_data.data, 0, LOG_DATA_SIZE);
	size = get_log_size();

	fread(hwvreg_data.data, size, 1, log_fp);

	fclose(log_fp);

	/*write_norflash_data */
	ret = write_norflash_data(&hwvreg_data);
	if (ret < 0) {
		printf("(%s:%d): write_norflash_data error \n", __func__, __LINE__);
	}

	log_fp = NULL;

	/* prepare next log_file */
	cdc_log_open();
	return 0;

}

static void *cdc_log_process(void *arg)
{
	int size = 0;
	prctl(PR_SET_NAME, "cdc_log_process");

	while (1) {
		size = get_log_size();
		if (size > LOG_DATA_SIZE) {
			cdc_log_write();
		}

		sleep(1);
	}

	return NULL;
}

void cdc_log_init()
{

	memset(&hwvreg_data, 0, sizeof(hwvreg_write_data));
	hwvreg_data.offset = LOGDATA_OFFSET;
	hwvreg_data.len = LOG_DATA_SIZE;
	hwvreg_data.data = malloc(hwvreg_data.len);
	if (!hwvreg_data.data) {
		printf("ERROR(%s): malloc log_data error! \n", __func__);
		return;
	}

	cdc_log_open();

	memset(hwvreg_data.data, 0, LOG_DATA_SIZE);

	/*create thread */
	int ret = -1;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	ret = pthread_create(&pid, &attr, &cdc_log_process, NULL);
	if (ret) {
		printf("ERROR(%s): cdc_log_process!\n", __func__);
		return ;
	}
}

void cdc_log_deinit()
{
	fclose(log_fp);
	log_fp = NULL;
	free(hwvreg_data.data);
	hwvreg_data.data = NULL;
	return;
}
