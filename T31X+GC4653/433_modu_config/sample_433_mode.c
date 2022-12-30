/*********************************************************
 * File Name   : sample_hx1838.c
 * Author      : Ipseven Chen
 * Mail        : Ipseven Chen@ingenic.com
 * Created Time: 2022-06-15 14:18
 说明：433模块测试   按键键值  与  延迟时间
 ********************************************************/

#include <stdio.h>
#include <sys/time.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char **argv)
{
	FILE *fp;
	struct input_event ie;
	struct input_event ie_pre;

	unsigned long delta;
	
	/*
		433模块在电路设计上，会发 5 次
		编码值，但我们需要一次，需要测出
		延时，将其过滤掉。
	*/
	int flag = 0;

	fp = fopen("/dev/input/event0", "r");
	if (fp == NULL) {
		perror(argv[1]);
		exit(1);
	}
	memset(&ie_pre, 0, sizeof(struct input_event));
	while (1) {
		fread((void *)&ie, sizeof(ie), 1, fp);
		if (ferror(fp)) {
			perror("fread");
			exit(1);
		}
		
		/*
			过滤掉多余的按键事件 与 flag 二选一
		*/
		/*if ( ie.type != EV_MSC ) {
			continue;
		} */
		
		printf("[##################]");

		switch(ie.value) {
			
			case 0x80:  //KEY_0
			case 0x50:  //KEY_1
			case 0x10:  //KEY_2
			case 0x20:  //KEY_3
			case 0x28:  //KEY_4
			case 0x18:  //KEY_5
			case 0x38:  //KEY_6
			case 0x40:  //KEY_7
			case 0x48:  //KEY_8
			case 0x58:  //KEY_9
			case 0x60:  //KEY_UP
			case 0xa0:  //KEY_DOWN
			case 0x30:  //KEY_LEFT
				delta = ie.time.tv_sec * 1000 + ie.time.tv_usec / 1000 - (ie_pre.time.tv_sec * 1000 + ie_pre.time.tv_usec / 1000);
				
				flag++;
				/*
					测试两次触发的input event 事件的间隔时间
					input_type :4----->复杂事件（先触发）
					input_type :1----->按键事件 (后触发)
				*/
				if( flag == 2) {
					flag = 0;
					printf("[INFO] time %d\n",delta);
				}

				/*
					需要的延迟时间
				*/
				if(delta > 200) {

					printf("[timeval:sec:%d,usec:%d,type:%d,code:%d,value:%d]\n",
							ie.time.tv_sec, ie.time.tv_usec,
							ie.type, ie.code, ie.value);	
					memcpy(&ie_pre, &ie, sizeof(struct input_event));
				}

				break;
			default:
				//other keyval, do none
				break;
		}

	}
	return 0;
}
