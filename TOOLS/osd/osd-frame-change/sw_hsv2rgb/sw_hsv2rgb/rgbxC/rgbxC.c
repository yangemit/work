#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	int i = 1;
	int fd_r = -1, fd_g = -1, fd_b = -1, rgbx = -1;
	fd_r = open("../bgr_r", O_RDONLY);
	fd_g = open("../bgr_g", O_RDONLY);
	fd_b = open("../bgr_b", O_RDONLY);
	rgbx = open("./rgbx", O_CREAT | O_RDWR);
	if ((fd_r <= 0) || (fd_g <= 0) || (fd_b <= 0) || (rgbx <= 0))
	{
		printf("open error! fd_r = %d, fd_g = %d, fd_b = %d, rgbx = %d\n", fd_r, fd_g, fd_b, rgbx);
		return -1;
	}
	unsigned char buf_r[1] = {0};
	unsigned char buf_g[1] = {0};
	unsigned char buf_b[1] = {0};
	unsigned char buf_x[1] = {0};
	unsigned char buf[4] = {0};

	int readc = 0;
	for (i = 0; i < 300*(102400); i++) {
		readc = read(fd_r, buf_r, 1);
		if (readc <= 0) {
			printf("read fdr == 0\n");
			goto end;
		}
		readc = read(fd_g, buf_g, 1);
		readc = read(fd_b, buf_b, 1);
		buf_x[0] = 0xff;
		/*printf("r = %x, g=%x, b=%x, a=%x\n", buf_r[0], buf_g[0], buf_b[0], buf_x[0]);*/
		buf[0] = buf_b[0];
		buf[1] = buf_g[0];
		buf[2] = buf_r[0];
		buf[3] = buf_x[0];
		readc = write(rgbx, buf, 4);
	}
end:
	close(fd_r);
	close(fd_g);
	close(fd_b);
	close(rgbx);

	return 0;
}
