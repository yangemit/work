/*********************************************************
 * File Name   : uart.h
 * Author      : Jasmine
 * Mail        : jian.dong@ingenic.com
 * Created Time: 2021-01-21 11:22
 ********************************************************/
#ifndef _UART_H__
#define _UART_H__
#include <stdio.h>
#include <linux/ioctl.h>
#include <linux/types.h>

typedef struct uart_agreement_t {
	uint8_t head;
	uint16_t length;
	uint8_t static_word;
	uint8_t command;
	uint8_t parameter;
	uint8_t check_sum;
}__attribute__((packed)) Uart_Agreement;

typedef struct uart_agreement_write_sn_t {
	uint8_t head;
	uint16_t length;
	uint8_t static_word;
	uint8_t command;
	uint8_t parameter;
	uint8_t check_sum;
	uint8_t sn[16];
}__attribute__((packed)) Uart_Agreement_Write_Sn;

typedef struct uart_agreement_ack_t {
	uint8_t head;
	uint16_t length;
	uint8_t static_word;
	uint8_t command;
	uint16_t parameter;
	uint8_t check_sum;
}__attribute__((packed)) Uart_Agreement_Ack;

struct uvc_hsb_string{
	const char *s;
};

struct led_status_t{
	int gpio;
	int status;
};

enum send_types_s
{
	ACK_FLAG,
	CONTROL_FLAG,
};


int sn_contrl_read(char *read_buf, int *data);
int sn_contrl_write(char *sn_buf);
int kiva_version_read(char *data, int *length);

#define UART0				"/dev/ttyS0"
#define UART2				"/dev/ttyS2"
#define CONFIG_FILE 			"/system/config/uvc.config"
#define KIVA_VERSION_PATH 	        "/tmp/version.txt"

#define USB0_DET                         57
#define USB2_DET                         58
#define USB_SET                          63

#define ACK_FLAG			0


#define UART0_NUM			1
#define UART2_NUM			2

#define Uart_Agreement_LEN		sizeof(Uart_Agreement)
#define Uart_Agreement_Ack_LEN		sizeof(Uart_Agreement_Ack)

#define NORMAL_LEN			3
#define ACK_LEN				4
#define WRITE_SN_LEN 			16

#define UART_HEAD			0xAA
#define UART_LENGTH			0x03
#define UART_LENGTH_ACK			0x04
#define UART_PARAMETER			0x00
//static_word
#define UART_STATIC_NORMAL_WORD			0xAA
#define UART_STATIC_WRITE_SN_WORD		0xAC
#define UART_STATIC_READ_SN_WORD		0xAB
#define UART_STATIC_ENTER_KEY_WORD		0xAD
#define UART_STATIC_ENTER_LED_WORD		0xAE
#define UART_STATIC_VERSION_WORD		0xAF
#define UART_STATIC_MAC_WORD			0xB0
/*LVC01 send LVC03 command */
#define COMMAND_ACK 			0x00
#define COMMAND_POWER_ON		0x01
#define COMMAND_POWER_OFF		0x02
#define COMMAND_CHANGE_VIEW_MODE	0x03
#define COMMAND_CALL_ON			0x04
#define COMMAND_ACK 			0x00
#define COMMAND_POWER_ON		0x01
#define COMMAND_POWER_OFF		0x02
#define COMMAND_CHANGE_VIEW_MODE	0x03
#define COMMAND_CALL_ON			0x04
#define COMMAND_CALL_OFF		0x05
#define COMMAND_MIC_MUTE		0x06
#define COMMAND_MIC_UNMUTE		0x07
#define COMMAND_SET_FRAME		0x08
#define COMMAND_SET_FPS			0x09
#define COMMAND_SCREEN_MOVE		0x0b
#define COMMAND_SCREEN_ZOOM		0x0c
#define COMMAND_USB_CONNECT_STATE	0x0d

#define STATUS_OK			0x00
#define STATUS_FAIL			0x01

#define UART_CHECK_NUM                  0xFB
#define UART_CHECK_NUM_ACK              0xFA



#endif

