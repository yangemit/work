#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

struct BGR//opencv中存储的是BGR图像
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
};

struct HSV//定义hsv结构体
{
	float h;
	float s;
	float v;
};

struct BGR *bgr = NULL;
struct HSV *hsv = NULL;

//RGB(BGR: 0~255)转HSV(H: [0~360), S: [0~1], V: [0~1])
void HSV2BGR(struct HSV *hsv,struct BGR *bgr)
{
	float h = (float)hsv->h;//
	float s = (float)(hsv->s)/255;
	float v = (float)hsv->v;
	unsigned int b = 0;
	unsigned int g = 0;
	unsigned int r = 0;

	/*printf("h = %d, s = %d, v =%d\n", h, s, v);*/
	int flag = ( int)( h/60 );
	float f = h/60 - flag;
	float p = v * (1 - s);
	float q = v * (1 - f * s);
	float t = v * (1 - (1 - f) * s);

	/*printf("f = %f, s = %f\n", f, s);*/

	switch ( flag )//公式算法
	{
		case 0:
			b = p;
			g = t;
			r = v;
			break;
		case 1:
			b = p;
			g = v;
			r = q;
			break;
		case 2:
			b = t;
			g = v;
			r = p;
			break;
		case 3:
			b = v;
			g = q;
			r = p;
			break;
		case 4:
			b = v;
			g = p;
			r = t;
			break;
		case 5:
			b = q;
			g = p;
			r = v;
			break;
		default:
			break;
	}

	int blue;
	int green;
	int red;
	/*printf("r = %d, g = %d, b =%d\n", r, g, b);*/
	/*blue = (int)( b * 255);*/
	blue = (int)( b );
	bgr->b = blue;

	/*green = (int)( g * 255);*/
	green = (int)( g );
	bgr->g = green;

	/*red = (int)( r * 255);*/
	red = (int)( r );
	bgr->r = red;
/*
 *    blue = (int)( b );
 *    bgr->b = ( blue > 255) ? 255 : blue;//大于255就取255，小于0就取0
 *    bgr->b = ( blue < 0) ? 0 : bgr->b;
 *
 *    [>green = (int)( g * 255);<]
 *    green = (int)( g );
 *    bgr->g = ( green > 255) ? 255 : green;
 *    bgr->g = ( green < 0) ? 0 : bgr->g;
 *
 *    [>red = (int)( r * 255);<]
 *    red = (int)( r );
 *    bgr->r = ( red > 255) ? 255 : red;
 *    bgr->r = ( red < 0) ? 0 : bgr->r;
 */
}

int main()
{
	int hsv_h = -1, hsv_s = -1, hsv_v = -1;
	int bgr_b = -1, bgr_g = -1, bgr_r = -1;
	int i = 0;

	hsv_h = open("./dsthsv_h.dat",O_RDONLY, 0666);
	if (hsv_h <= 0) {
		printf("open dsthsv_rgba2hsv_640x480_h.dat error! hsv_h = %d\n", hsv_h);
		return -1;
	}
	hsv_s = open("./dsthsv_s.dat",O_RDONLY, 0666);
	if (hsv_s <= 0) {
		printf("open dsthsv_rgba2hsv_640x480_s.dat error! hsv_s = %d\n", hsv_s);
		return -1;
	}
	hsv_v = open("./dsthsv_v.dat",O_RDONLY, 0666);
	if (hsv_v <= 0) {
		printf("open dsthsv_rgba2hsv_640x480_v.dat error! hsv_v = %d\n", hsv_v);
		return -1;
	}
	bgr_b = open("./bgr_b", O_CREAT | O_WRONLY, 0666);
	bgr_g = open("./bgr_g", O_CREAT | O_WRONLY, 0666);
	bgr_r = open("./bgr_r", O_CREAT | O_WRONLY, 0666);
	if ((bgr_b&bgr_g&bgr_r) < 0) {
		printf("open bgr error\n");
		return -1;
	}

	hsv = (struct HSV *)malloc(sizeof(struct HSV));
	bgr = (struct BGR *)malloc(sizeof(struct BGR));
	int h[1] = {0};
	int s[1] = {0};
	int v[1] = {0};
	int r[1] = {0};
	int g[1] = {0};
	int b[1] = {0};
	int rcount = 0;
	for (i = 0; i < 10*1024*1024; i++) {
		rcount = read(hsv_h, h, 2);
		if (rcount <= 0) {
			printf("read hsv_h error, rcount = %d, %d\n", rcount, __LINE__);
				return -1;
		}
		/*printf("read hsv_h h =%d\n", h[0]);*/
		rcount = read(hsv_s, s, 1);
		if (rcount <= 0) {
			printf("read hsv_s error, rcount = %d, %d\n", rcount, __LINE__);
				return -1;
		}
		/*printf("read hsv_s s =%d\n", s[0]);*/
		rcount = read(hsv_v, v, 1);
		if (rcount <= 0) {
			printf("read hsv_v error, rcount = %d, %d\n", rcount, __LINE__);
				return -1;
		}
		/*printf("read hsv_v v =%d\n", v[0]);*/
		hsv->h = (float)h[0];
		hsv->s = (float)s[0];
		hsv->v = (float)v[0];
		HSV2BGR(hsv, bgr);
		r[0] = bgr->r;
		g[0] = bgr->g;
		b[0] = bgr->b;
		/*printf("r = %d, g = %d, b =%d\n", bgr->r, bgr->g, bgr->b);*/
		/*printf("r = %d, g = %d, b =%d\n", r[0], g[0], b[0]);*/
		rcount = write(bgr_r, r, 1);
		rcount = write(bgr_g, g, 1);
		rcount = write(bgr_b, b, 1);
	}

	close(hsv_h);
	close(hsv_s);
	close(hsv_v);
	close(bgr_b);
	close(bgr_g);
	close(bgr_r);
	printf("finish.....\n");

	return 0;
}
