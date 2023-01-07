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

#include <AntiCopy_Verify.h>

#define KEY_LEN         16
#define CODE_LEN        KEY_LEN
#define CRC_LEN		1
#define R0_LEN		16
#define R1_LEN		24

#define MAGIC_LEN       8
#define MAGIC           "ANTICOPY"

#define ANTICOPY_PROCESS_NAME "anticopy"

/**
 * total 64 Bytes
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

#define GLOBAL_KEY   "1234567890123456"

extern int get_chip_id(unsigned char *chipCodeId);

int gen_license(license_t *license)
{
	int i, ret;
	unsigned char key[16];
	unsigned char chip_id[16];

	strncpy((char *)&key, GLOBAL_KEY, KEY_LEN);

	ret = get_chip_id(chip_id);
	if (ret) {
		return -1;
	}

	strncpy((char *)license->magic, MAGIC, MAGIC_LEN);

	for (i = 0; i < R0_LEN / sizeof(int); i++) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		srand((unsigned int)tv.tv_usec);
		((unsigned int *)&(license->r0))[i] = (unsigned int)rand();
	}

	for (i = 0; i < KEY_LEN; i++) {
		license->code[i] = (chip_id[i] ^ (~key[i])) + 'a';
	}

	for (i = 0; i < (R1_LEN + CRC_LEN) / sizeof(int); i++) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		srand((unsigned int)tv.tv_usec);
		((unsigned int *)&(license->r1))[i] = (unsigned int)rand();
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

	char *process_path = argv[1];

	printf("process_path = [%s]\n", process_path);

	if (!access(process_path, R_OK)) {
		license_t license = {
#ifdef ANTICOPY_PREV_STRING
			.prev = ANTICOPY_PREV_STRING,
#endif
		};

		ret = gen_license(&license);
		if (ret) {
			printf("ERROR: gen license failed!\n");
			exit(1);
		}

		/* fill license */
		FILE *fp = fopen(process_path, "ab");
		if (!fp) {
			printf("ERROR: can't open file to write! %s\n", strerror(errno));
			exit(1);
		}

		int wsize = fwrite((void *)&license, 1, sizeof(license_t), fp);
		if (wsize != sizeof(license_t)) {
			printf("ERROR: can't write file! %s\n", strerror(errno));
			fclose(fp);
			exit(1);
		}

		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
	} else {
		printf("ERROR: can't find process file! %s\n", strerror(errno));
		exit(1);
	}

	return 0;
}
