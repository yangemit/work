#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <AntiCopy_Verify.h>

#define KEY_LEN         16
#define CODE_LEN        KEY_LEN
#define CRC_LEN		1
#define R0_LEN		16
#define R1_LEN		24

#define MAGIC_LEN       8
#define MAGIC           "ANTICOPY"

#define ANTICOPY_PROCESS_NAME "anticopy"
#define GLOBAL_KEY   "1234567890123456"

/**
 * total 128 Bytes
 **/
typedef struct _license {
	unsigned char magic[MAGIC_LEN];
	unsigned char r0[R0_LEN];
	unsigned char code[KEY_LEN];
	unsigned char r1[R1_LEN];
#ifdef ANTICOPY_PREV_STRING
	char prev[PREV_LEN];
#endif
} license_t;

extern int get_chip_id(unsigned char *chipCodeId);

#ifdef ANTICOPY_PREV_STRING
int verify_prev(license_t *license,char *prev) {

	printf("license->prev = %s\n", license->prev);
	if (strncmp(license->prev, prev, PREV_LEN)) {
		printf("********ERROR: prev false***********\n");
		return -1;
	}
	return 0;
}
#endif

int verify_license(license_t *license)
{
	int i, ret;
	unsigned char key[16];
	unsigned char chip_id[16];
	unsigned char code[KEY_LEN];

	strncpy((char *)&key, GLOBAL_KEY, KEY_LEN);

	ret = get_chip_id(chip_id);
	if (ret) {
		return -1;
	}

	for (i = 0; i < KEY_LEN; i++) {
		code[i] = (chip_id[i] ^ (~key[i])) + 'a';
	}

	for (i = 0; i < KEY_LEN; i++) {
		if (code[i] != license->code[i]) {
			return -1;
		}
	}

	return 0;

}

#ifndef ANTICOPY_PREV_STRING
int AntiCopy_Verify(void)
{
	int ret;

	char process_path[128];
	int len = readlink("/proc/self/exe", process_path, 128);
	if (len <= 0) {
		printf("ERROR: can't find my path! %s\n", strerror(errno));
		exit(1);
	}
	process_path[len] = '\0';

	char anticopy_path[128];
	strcpy(anticopy_path, process_path);

	char *path_end = strrchr(anticopy_path, '/');
	strcpy(path_end + 1, ANTICOPY_PROCESS_NAME);
	if (!access(anticopy_path, R_OK)) {
		ret = unlink(anticopy_path);
		if (ret) {
			printf("ERROR: can't remove anticopy file! %s\n", strerror(errno));
			exit(1);
		}
	}

	license_t license;

	FILE *fp = fopen(process_path, "rb");
	if (!fp) {
		printf("ERROR: can't open file for read! %s\n", strerror(errno));
		exit(1);
	}
	fseek(fp, 0 - sizeof(license_t), SEEK_END);

	int rsize = fread((void *)&license, 1, sizeof(license_t), fp);
	if (rsize != sizeof(license_t)) {
		printf("ERROR: read file failed! %s\n", strerror(errno));
		fclose(fp);
		exit(1);
	}
	fclose(fp);

	if (!strncmp((char *)&(license.magic), MAGIC, MAGIC_LEN)) {
		/* check license */
		ret = verify_license(&license);
		if (ret) {
			printf("ERROR: anticopy verify failed!\n");
			exit(1);
		}
		printf("INFO: anticopy verify OK!\n");
	} else {
		printf("ERROR: anticopy verify failed!\n");
		exit(1);
	}

	return 0;

}
#else
int AntiCopy_Verify_PrevStr(char *prev)
{
	int ret;
	char process_path[128];
	int len = readlink("/proc/self/exe", process_path, 128);
	if (len <= 0) {
		printf("ERROR: can't find my path! %s\n", strerror(errno));
		exit(1);
	}
	process_path[len] = '\0';

	char anticopy_path[128];
	strcpy(anticopy_path, process_path);

	char *path_end = strrchr(anticopy_path, '/');
	strcpy(path_end + 1, ANTICOPY_PROCESS_NAME);
	if (!access(anticopy_path, R_OK)) {
		ret = unlink(anticopy_path);
		if (ret) {
			printf("ERROR: can't remove anticopy file! %s\n", strerror(errno));
			exit(1);
		}
	}

	license_t license;
	memset(&license, 0, sizeof(license_t));

	FILE *fp = fopen(process_path, "rb");
	if (!fp) {
		printf("ERROR: can't open file for read! %s\n", strerror(errno));
		exit(1);
	}
	fseek(fp, 0 - sizeof(license_t), SEEK_END);

	int rsize = fread((void *)&license, 1, sizeof(license_t), fp);
	if (rsize != sizeof(license_t)) {
		printf("ERROR: read file failed! %s\n", strerror(errno));
		fclose(fp);
		exit(1);
	}
	fclose(fp);

	if (!strncmp((char *)&(license.magic), MAGIC, MAGIC_LEN)) {
		/* check license */
		ret = verify_license(&license);
		if (ret) {
			printf("ERROR: anticopy verify failed, license error!\n");
			exit(1);
		}

		/*check prev str*/
	        ret = verify_prev(&license,prev);
		if (ret) {
			printf("ERROR: anticopy verify_prev failed, maybe prev str error!\n");
			exit(1);
		}

	} else {
		printf("ERROR: anticopy verify failed, magic not match!\n");
		exit(1);
	}
	printf("INFO: anticopy verify OK!");

	return 0;

}
#endif

