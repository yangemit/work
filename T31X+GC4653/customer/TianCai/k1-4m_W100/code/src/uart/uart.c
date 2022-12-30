/*********************************************************
 * File Name   : uart.c
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2021-03-05 12:05
 ********************************************************/

#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <termios.h>

#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <imp-common.h>

#include "uart.h"
#include "usbcamera.h"


enum uart_mutex_s
{
	UART_MUTEX_TYPE_SEND_MESSAGE,
	UART_MUTEX_TYPE_SEND_ACK,
	UART_MUTEX_TYPE_NUM
	
};

static struct led_status_t led_status[3] = 
{
	{49,0},//rea	
	{50,0},//green	
	{59,0},//blue	
};

pthread_mutex_t send_message_mutex[UART_MUTEX_TYPE_NUM] = {0};

Uart_Agreement uart_send_data = {
	.head 	= 0xAA,
	.length = 0x00,
	.static_word = 0xAA,
	.command = 0x00, 
	.parameter = 0x00,
	.check_sum = 0x00,
};

Uart_Agreement_Ack uart_send_ack_data = {
	.head 	= 0xAA,
	.length = 0x00,
	.static_word = 0xAA,
	.command = 0x00, 
	.parameter = 0x00,
	.check_sum = 0x00,
};

static int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
	struct termios newtio,oldtio;
	if  ( tcgetattr( fd,&oldtio)  !=  0) {
		perror("SetupSerial 1");
		return -1;
	}
	bzero( &newtio, sizeof( newtio ) );
	newtio.c_cflag  |=  CLOCAL | CREAD;
	newtio.c_cflag &= ~CSIZE;

	switch( nBits ) {
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	switch( nEvent ) {
		case 'O':
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;
		case 'E':
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;
		case 'N':
			newtio.c_cflag &= ~PARENB;
			break;
	}

	switch( nSpeed ) {
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 4800:
			cfsetispeed(&newtio, B4800);
			cfsetospeed(&newtio, B4800);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
		case 230400:
			cfsetispeed(&newtio, B230400);
			cfsetospeed(&newtio, B230400);
			break;

		case 460800:
			cfsetispeed(&newtio, B460800);
			cfsetospeed(&newtio, B460800);
			break;
		case 921600:
			cfsetispeed(&newtio, B921600);
			cfsetospeed(&newtio, B921600);
			break;
		default:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
	}
	if( nStop == 1 )
		newtio.c_cflag &=  ~CSTOPB;
	else if ( nStop == 2 )
		newtio.c_cflag |=  CSTOPB;
	newtio.c_cc[VTIME]  = 0;
	newtio.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH);
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("com set error");
		return -1;
	}
	return 0;
}


static int usb0_status = 1, usb2_status = 1;

static int uart_select()
{
	int fd = -1, i = 0;
	sample_ucamera_gpio_in_init(USB0_DET);
	sample_ucamera_gpio_in_init(USB2_DET);
	while(i < 20)
	{
		usb0_status = sample_ucamera_gpio_read(USB0_DET);
		printf("usb0_status=%d\n",usb0_status);
		if(usb0_status == 0){
			usleep(5 * 1000);
			usb0_status = sample_ucamera_gpio_read(USB0_DET);
			if(usb0_status == 0){
				sample_ucamera_led_init(USB_SET, 0);
				/*fd = open(UART0, O_RDWR|O_NOCTTY|O_NDELAY, 0666);*/
				fd = open(UART0, O_RDWR|O_NOCTTY, 0666);
				if(fd < 0){
					Ucamera_LOG("%s is open failed\n",UART0);
					return -1;
				} else{
					Ucamera_LOG("open is uart %s",UART0);
					return fd;
				}
			}
		} else if(usb0_status == 1){
			usb2_status = sample_ucamera_gpio_read(USB2_DET);
			printf("usb2_status=%d\n",usb2_status);
			if(usb2_status == 0){
				usleep(5 * 1000);
				usb2_status = sample_ucamera_gpio_read(USB2_DET);
				if(usb2_status == 0){
					sample_ucamera_led_init(USB_SET, 1);
					printf("########USB_SET=1#######\n");
					/*fd = open(UART2, O_RDWR|O_NOCTTY|O_NDELAY, 0666);*/
					fd = open(UART2, O_RDWR|O_NOCTTY, 0666);
					if(fd < 0){
						Ucamera_LOG("%s is open failed\n",UART2);
						return -1;
					} else {
						Ucamera_LOG("open is uart %s",UART2);
						return fd;
					}
				}

			}
		}
		usleep(50 * 1000);
		i++;
	}
	return -1;
}

static int uart_fd = -1;


static int uart_info_send(char *data_buf, int data_len)
{
	int w_len = 0;
	w_len = write(uart_fd, data_buf, data_len);
	if (w_len != data_len){	
		printf("uart send data is failed\n");
		return -1;
	}
	return 0;
}

int uart_send_message(uint8_t command, uint8_t parameter)
{
	pthread_mutex_lock(&send_message_mutex[UART_MUTEX_TYPE_SEND_MESSAGE]);
	char data_buf[64] = {0};
	uart_send_data.command = command;
	uart_send_data.parameter = parameter;
	uart_send_data.length = sizeof(uart_send_data.static_word) + sizeof(uart_send_data.command) + sizeof(uart_send_data.parameter);
	uart_send_data.check_sum = 0xFF - (uart_send_data.length + uart_send_data.static_word + uart_send_data.command + uart_send_data.parameter) + 1;
	uart_send_data.length = uart_send_data.length << 8;
	/*printf("uart_send_data.length=%d\n"uart_send_data.length;*/
	memcpy(data_buf, (void *)&uart_send_data, sizeof(uart_send_data));
	uart_info_send(data_buf, sizeof(uart_send_data));
	pthread_mutex_unlock(&send_message_mutex[UART_MUTEX_TYPE_SEND_MESSAGE]);
	return 0;
}

int uart_send_ack(uint8_t command, uint16_t status)
{
	pthread_mutex_lock(&send_message_mutex[UART_MUTEX_TYPE_SEND_ACK]);
	char data_buf[64] = {0};

	uart_send_ack_data.parameter = (status << 8) + command;

	uart_send_ack_data.length = sizeof(uart_send_ack_data.static_word) + sizeof(uart_send_ack_data.command) + sizeof(uart_send_ack_data.parameter);

	uart_send_ack_data.check_sum = 0xFF - (uart_send_ack_data.length + uart_send_ack_data.static_word + uart_send_ack_data.command + uart_send_ack_data.parameter) + 1;
	uart_send_ack_data.length = uart_send_ack_data.length << 8;
	printf("uart_send_ack_data.length=%d,parameter=%0x\n",uart_send_ack_data.length,uart_send_ack_data.parameter);
	memcpy(data_buf, (void *)&uart_send_ack_data, sizeof(uart_send_ack_data));
	uart_info_send(data_buf, sizeof(uart_send_ack_data));
	pthread_mutex_unlock(&send_message_mutex[UART_MUTEX_TYPE_SEND_ACK]);
	return 0;
}

static int normal_command(uint8_t command,uint8_t parameter)
{
	/*int ret = -1;*/
	uint16_t status = 0,value;
	static int zoom_value = 0, move_value = 5;
	/*char *data_buf = "AABBCCDD";  */
	/*uart_info_send(data_buf, strlen(data_buf));*/
	switch(command){
		case COMMAND_SCREEN_ZOOM: 
			if(parameter == 0) {
				zoom_value++;
				if(zoom_value > 10)
					zoom_value = 10;
			} else if(parameter == 1) {
				zoom_value--;
				if(zoom_value < 0)
					zoom_value = 0;
			}
			value = 100 + zoom_value * 10;
			sample_video_zoom_set(value);
			/*sample_video_zoom_move_set(value, move_value);*/
			break;
		case COMMAND_SCREEN_MOVE: 
			move_value = parameter;
			/*sample_video_zoom_move_set(value, move_value);*/
			break;
		case COMMAND_USB_CONNECT_STATE:
			if(parameter == 0){
				sample_ucamera_led_ctl(led_status[0].gpio, 1);
				sample_ucamera_led_ctl(led_status[1].gpio, 0);
				sample_ucamera_led_ctl(led_status[2].gpio, 0);
			} else if(parameter == 1){
				sample_ucamera_led_ctl(led_status[0].gpio, 0);
				sample_ucamera_led_ctl(led_status[1].gpio, 1);
				sample_ucamera_led_ctl(led_status[2].gpio, 0);
			}
			break;
	}
	printf("normal_command=%d,parameter=%d,value=%d\n",command, parameter,value);
	uart_send_ack(command,status);
	return 0;
}

static int led_status_flags = 0;
static sem_t led_ready_sem;

static void *led_function_test(void *args)
{
	while(led_status_flags){
	sample_ucamera_led_ctl(led_status[0].gpio, 0);
	sample_ucamera_led_ctl(led_status[1].gpio, 0);
	sample_ucamera_led_ctl(led_status[2].gpio, 1);
	sleep(1);
	sample_ucamera_led_ctl(led_status[0].gpio, 0);
	sample_ucamera_led_ctl(led_status[1].gpio, 1);
	sample_ucamera_led_ctl(led_status[2].gpio, 0);
	sleep(1);
	sample_ucamera_led_ctl(led_status[0].gpio, 1);
	sample_ucamera_led_ctl(led_status[1].gpio, 0);
	sample_ucamera_led_ctl(led_status[2].gpio, 0);
	sleep(1);
	}
	sem_post(&led_ready_sem);
	return NULL;
}

static int production_test_command(uint8_t static_word)
{
	int ret = -1,i = 0;
	char data[32] = {0};
	char error[8] = "FAIL\n";
	char led_begin[4] = "1\n";
	char led_end[4] = "0\n";
	int length = -1;
	pthread_t tid_led;
	pthread_attr_t attr;
	switch (static_word){
		case UART_STATIC_READ_SN_WORD:
			ret = sn_contrl_read(data, &length);
			printf("read sn data=%s,length=%d\n",data,length);
			break;
		case UART_STATIC_ENTER_KEY_WORD:
			printf("enter key static_word=%d\n",static_word);
			break;
		case UART_STATIC_ENTER_LED_WORD:
			led_status_flags = ~led_status_flags; 
			if(led_status_flags != 0){
				printf("enter led test\n");
				for(i = 0; i < 3; i++)
					led_status[i].status = sample_ucamera_gpio_read(led_status[i].gpio);
				uart_info_send(led_begin, 1);
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
				ret = pthread_create(&tid_led, &attr, led_function_test, NULL);
				if(ret != 0)
				{
					printf("pthread cread led_function_test is failed\n");
					return -1;
				}

			} else if(led_status_flags == 0){
				sem_wait(&led_ready_sem);

				for(i = 0; i < 3; i++)
					sample_ucamera_led_ctl(led_status[i].gpio, led_status[i].status);
				uart_info_send(led_end, 1);
				printf("quit led test\n");
			}
			return 0;
			break;
		case UART_STATIC_VERSION_WORD:
			ret = kiva_version_read(data, &length);
			printf("read version data=%s,length=%d\n",data,length);
			break;
		case UART_STATIC_MAC_WORD:
			printf("read mac\n");
			return 0;
			break;

	}
	if(ret == 0)
		uart_info_send(data, length);
	else
		uart_info_send(error, strlen(error));

	return 0;
}

/*
 *static int write_sn_command(char *sn)
 *{
 *        printf("write_sn_command,sn=%s\n",sn);
 *        if(static_word != UART_STATIC_READ_SN_WORD){
 *                printf("static_word if error %d\n",static_word);
 *                return -1;
 *        }
 *        sn_contrl_write(sn);
 *        return 0;
 *}
 */

static int uart_recive(int fd)
{
	char read_buf[32] = {0};
	int rsize;
	int uart_fd = -1, ret = -1;
	int retval = -1, total_len = 0;
	Uart_Agreement *uart_recive_agree = NULL;
	Uart_Agreement_Write_Sn *uart_write_sn_recive_agree = NULL;
	/*Uart_Agreement_Ack *uart_recive_ack_agree = NULL; */
	uart_recive_agree = malloc(sizeof(Uart_Agreement));
	uart_write_sn_recive_agree = malloc(sizeof(Uart_Agreement_Write_Sn));
	/*uart_recive_ack_agree = malloc(sizeof(Uart_Agreement_Ack));*/
	uart_fd = fd;
	fd_set  rfds;
	char success[4] = "OK\n";
	char error[8] = "FAIL\n";
	set_opt(uart_fd, 115200, 8, 'N', 1);
	while(1){
		/*
		 *rsize = 0;
		 *memset(read_buf, '0', sizeof(read_buf));
		 *rsize = read(fd, read_buf, sizeof(read_buf)); 
		 *if(rsize > 0){
		 *        printf("########rsize len =%d#########\n",rsize);
		 *                uart_recive_agree = (Uart_Agreement *)read_buf;
		 *                printf("head=0x%x,length=0x%x,static_word=0x%x,command=0x%x,parameter=0x%x,check_sum=0x%x\n",uart_recive_agree->head,uart_recive_agree->length,uart_recive_agree->static_word,uart_recive_agree->command,uart_recive_agree->parameter,uart_recive_agree->check_sum);
		 *}
		 */
		FD_ZERO(&rfds);
		FD_SET(uart_fd, &rfds);
		retval = select(uart_fd + 1, &rfds, NULL, NULL, NULL);
		if (retval == -1 && errno == EINTR)
			continue;
		if(retval < 0){
			perror("select");
			return -1;
		}

		if(FD_ISSET(uart_fd, &rfds)){
			rsize = 0;
			memset(read_buf, 0, sizeof(read_buf));
			memset(uart_write_sn_recive_agree, 0, sizeof(Uart_Agreement_Write_Sn));
			memset(uart_recive_agree, 0, sizeof(Uart_Agreement));

			rsize = read(uart_fd, read_buf, sizeof(read_buf)); 
			if(rsize > 0){
				printf("########rsize len =%d,%s#########\n",rsize,read_buf);
				uart_write_sn_recive_agree = (Uart_Agreement_Write_Sn *)read_buf;
				printf("uart_write_sn_recive_agree->head=%d,static_word=%d\n",uart_write_sn_recive_agree->head,uart_write_sn_recive_agree->static_word);
				if(uart_write_sn_recive_agree->head != UART_HEAD){
					printf("uart head error= %d\n",uart_write_sn_recive_agree->head);
					continue;
				}

				if(uart_write_sn_recive_agree->static_word != UART_STATIC_WRITE_SN_WORD){
					uart_recive_agree = (Uart_Agreement *)read_buf;
					/*printf("head=0x%x,length=0x%x,static_word=0x%x,command=0x%x,parameter=0x%x,check_sum=0x%x\n",uart_recive_agree->head,uart_recive_agree->length,uart_recive_agree->static_word,uart_recive_agree->command,uart_recive_agree->parameter,uart_recive_agree->check_sum);*/
					if(uart_recive_agree->static_word == UART_STATIC_NORMAL_WORD && uart_recive_agree->command == 0){
					/*ack_command*/
						printf("ack_command\n");
						continue;
					}

					uart_recive_agree->length = uart_recive_agree->length >> 8; 
					if(uart_recive_agree->length != NORMAL_LEN){ 
						printf("uart_recive_agree length if error length=%d\n",uart_recive_agree->length);
						continue;
					}

					total_len = 0xff - (uart_recive_agree->length + uart_recive_agree->static_word + uart_recive_agree->command + uart_recive_agree->parameter) + 1;
					if(uart_recive_agree->check_sum != total_len){ 
						printf("uart_recive_agree check_sum if error check_sum=%d,total_len=%d\n",uart_recive_agree->check_sum,total_len);
						continue;
					}

					/*
					 *if(uart_recive_agree->parameter != 0) { 
					 *        printf("uart_recive_agree parameter =%d error\n",uart_recive_agree->parameter);
					 *        continue;
					 *        }
					 */
					if(uart_recive_agree->static_word != UART_STATIC_NORMAL_WORD){
						if(uart_recive_agree->command != 0){ 
							printf("uart_recive_agree common =%d  error\n",uart_recive_agree->command);
							continue;
							}
							production_test_command(uart_recive_agree->static_word);
							continue;

					}
					normal_command(uart_recive_agree->command,uart_recive_agree->parameter);
					continue;
				} else {
					uart_write_sn_recive_agree->length = uart_write_sn_recive_agree->length >> 8; 
					if(uart_write_sn_recive_agree->length != 3){
						printf("uart_write_sn_recive_agree length=%d\n",uart_write_sn_recive_agree->length);
						uart_info_send(error, strlen(error));
						continue;
					}

					if(strlen((char *)uart_write_sn_recive_agree->sn) != WRITE_SN_LEN){
					printf("uart_write_sn_recive_agree->sn length is error sn=%s,length=%d\n",uart_write_sn_recive_agree->sn, strlen((char *)uart_write_sn_recive_agree->sn));
						uart_info_send(error, strlen(error));
						continue;
					}

					total_len = 0xff - (uart_write_sn_recive_agree->length + uart_write_sn_recive_agree->static_word) + 1;

					if(uart_write_sn_recive_agree->check_sum != total_len){ 
						printf("uart_write_sn_recive_agree check_sum if error check_sum=%d\n",uart_write_sn_recive_agree->check_sum);
						uart_info_send(error, strlen(error));
						continue;
					}
					ret = sn_contrl_write((char *)uart_write_sn_recive_agree->sn);
					if(ret == 0)
						uart_info_send(success, strlen(success));
					else
						uart_info_send(error, strlen(error));
	
				}
			} else { 
				if(!usb2_status)
					printf("read if uart %s failed\n",UART2);
				else
					printf("read if uart %s failed\n",UART0);
				perror("ERROR:");
				usleep(1000 * 1000);
			}
		}

	}
	return 0;
}

void *hid_event_thread(void *arg)
{
	int i = 0;
	prctl(PR_SET_NAME, "uart_event");
	sem_init(&led_ready_sem, 0, 0);
	uart_fd = uart_select();
	printf("#########uart_fd=%d########\n",uart_fd);
	if(uart_fd < 0){
		printf("uart_open is failed\n");
		return 0;
	}
	for(i = 0; i < UART_MUTEX_TYPE_NUM; i++)
		pthread_mutex_init(&send_message_mutex[i], NULL);
	/*uart_send_message(command, parameter);*/

	uart_recive(uart_fd);
	return 0; 

}

int uart_comm_entry()
{
	int ret;

	pthread_t uart_pid;
	pthread_attr_t uart_attr;
	pthread_attr_init(&uart_attr);
	pthread_attr_setdetachstate(&uart_attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setschedpolicy(&uart_attr, SCHED_OTHER);
	ret = pthread_create(&uart_pid, &uart_attr, &hid_event_thread, NULL);
	if (ret) {
		Ucamera_LOG("create thread for osd_show failed!\n");
		return -1;
	}

	return 0;
}

