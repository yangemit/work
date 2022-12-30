/**
 * test
 * jingbin.bi@ingenic.com
 * 2020-03-18
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/*#include "../inc/global_config.h"*/
#include "package_gen.h"

extern int file_md5sum_0(FILE *fp, unsigned char md5[16]);

static int copy_file_to_file(char *src_file, char *dst_file)
{
	int rsize = 0;

	/* alloc memory */
	void *rbuf = malloc(READ_SIZE_ONCE);
	if (!rbuf) {
		printf("ERROR: alloc memory failed\n");
		return -1;
	}

	FILE *src_fp = fopen(src_file, "rb");
	if (!src_fp) {
		printf("ERROR: open file [%s] failed\n", src_file);
		return -1;
	}

	FILE *dst_fp = fopen(dst_file, "a");
	if (!dst_fp) {
		printf("ERROR: open file [%s] failed\n", dst_file);
		return -1;
	}

	while ((rsize = fread(rbuf, 1, READ_SIZE_ONCE, src_fp))) {
		if (rsize < 0) {
			printf("ERROR: read file [%s] failed\n", src_file);
			return -1;
		}

		int wsize = 0, wtotal = 0;
		while (rsize - wtotal) {
			wsize = fwrite(rbuf + wtotal, 1, rsize - wtotal, dst_fp);
			if (wsize < 0) {
				printf("ERROR: write npu failed!\n");
				return -1;
			}
			wtotal += wsize;
		}
	}

	fflush(dst_fp);
	fsync(fileno(dst_fp));
	fclose(dst_fp);
	fclose(src_fp);
	free(rbuf);

	return 0;
}

static int calc_md5_to_file(char *src_file, unsigned char md5[16])
{

	FILE *src_fp = fopen(src_file, "rb");
	if (!src_fp) {
		printf("ERROR: open file [%s] failed\n", src_file);
		return -1;
	}

	file_md5sum_0(src_fp, md5);

	fclose(src_fp);

	return 0;
}

int main()
{
	int ret = 0;
	unsigned char md5sum[16];
	char	update_head[128];


	/* check and get file size */
	struct stat st;

	ret = stat(FIRMWARE_PATH, &st);
	if (ret) {
		printf("ERROR: get file stat of [%s] failed!\n", FIRMWARE_PATH);
		return -1;
	}
	int npu_firmware_size = st.st_size;;

	printf("firmware size         : %d Bytes / %d KB / %d MB \n",
	       npu_firmware_size, npu_firmware_size / 1024, npu_firmware_size / 1024 / 1024);

	/* write header */
	FILE *fw_fp = fopen(UPDATE_PACKAGE_PATH, "w+");
	if (!fw_fp) {
		printf("ERROR: create file [%s] failed\n", UPDATE_PACKAGE_PATH);
		return -1;
	}

	ret = calc_md5_to_file(FIRMWARE_PATH, md5sum);
	if (ret) {
		printf("ERROR: copy md5 of [%s] failed\n", FIRMWARE_PATH);
		return -1;
	}
	printf("#####md5sum[15]=%d#######\n",md5sum[15]);


	/* write header */
	firmware_head_t head;
	memset(&head, 0, sizeof(firmware_head_t));
	memset(update_head, 0, sizeof(update_head));

	head.version = KIVA_VERSION;
	head.img_offset_file = IMG_OFFSET;
	head.img_size	     = st.st_size;
	memcpy((void *)head.md5, (void *)md5sum, sizeof(md5sum));
	memcpy(update_head, &head, sizeof(firmware_head_t));

	int wsize;
	wsize = fwrite(update_head, 1, sizeof(update_head), fw_fp);
	if (wsize != sizeof(update_head)) {
		printf("ERROR: write head failed!\n");
		return -1;
	}

	fflush(fw_fp);
	fsync(fileno(fw_fp));
	fclose(fw_fp);

	ret = copy_file_to_file(FIRMWARE_PATH, UPDATE_PACKAGE_PATH);
	if (ret) {
		printf("ERROR: copy file [%s] failed\n", FIRMWARE_PATH);
		return -1;
	}

	return 0;
}
