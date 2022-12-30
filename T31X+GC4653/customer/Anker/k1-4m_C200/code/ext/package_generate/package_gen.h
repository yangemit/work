#ifndef __PACKAGE_GENERATE_H__
#define __PACKAGE_GENERATE_H__

#define FIRMWARE_PATH		"./firmware/firmware.bin"

#define KIVA_VERSION            0x1000                    /*V1.0*/
#define UPDATE_PACKAGE_PATH 	"./update_package.bin"
#define IMG_OFFSET		(32 * 1024)
#define READ_SIZE_ONCE		(1 * 1024 * 1024)

/* NOTE: little endian */
typedef struct _firmware_part_info {
	int32_t version;
	int32_t reserved0;
	int32_t size;
	int32_t reserved1;
	int32_t reserved2;
} firmware_part_info_t;

/**
 * struct size must be FIRMWARE_HEAD_SIZE
 **/
typedef struct _firmware_head {
	uint32_t version;
	uint32_t img_offset_file;
	uint32_t img_size;
	uint8_t md5[16];
	uint8_t priv[100];
} firmware_head_t;

#endif
