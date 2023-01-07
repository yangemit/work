#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("The input para is too few...\n");
		return -1;
	}
	printf("Your cmd is: %s\n", *argv);

	char diff_p[100] = {0};
	unsigned char buf1[1] = {0};
	unsigned char buf2[1] = {0};
	int i = 0;
	int number = 0, diff = 0;
	int c = 0;

	int fd1 = -1, fd2 = -1;
	fd1 = open(argv[1], O_RDONLY, 0666);
	fd2 = open(argv[2], O_RDONLY, 0666);

	for (i = 0; i < 1024*1024*1024; i++) {
		c = read(fd1, buf1, 1);
		if (c == 0 ) {
			printf("read c == 0\n");
			goto end;
		}
		c = read(fd2, buf2, 1);
		if ((i+1) % 4 != 0) {
			if (i == 10001)
				printf("----------------\n");
			diff = abs(buf1[0] - buf2[0]);
			if ((diff > 3) || (diff < -3)){
				/*diff_p[number] = diff;*/
				/*printf("buf1[0] = %d, buf2[0] = %d, diff = %d, i = %d\n",buf1[0], buf2[0], diff, i);*/
				number++;
			}
		}
	}

end:
	close(fd1);
	close(fd2);
	printf(" i = %d, number = %d, rate = %f\n", i, number, (float)number/(float)i);
	/*
	 *for (i = 0; i <= number; i++) {
	 *    printf("diff_p_%d = %d	", i, diff_p[i]);
	 *    if (i/5 == 0)
	 *        printf("\n");
	 *}
	 */

	return 0;
}
