/*
 * Boot support
 */
#include <common.h>


#ifdef CONFIG_LZO
#include <linux/lzo.h>
#else
#include <LzmaTypes.h>
#include <LzmaDec.h>
#include <LzmaTools.h>
#endif

//#define DEBUG_DATA
#define CHECK_HEAD
#define CHECK_DATA

#ifndef CONFIG_SYS_BOOTM_LEN
#ifdef MAX_U_IMAGE_LIMIT
#define CONFIG_SYS_BOOTM_LEN	((MAX_U_IMAGE_LIMIT) * 3)
#else /* MAX_U_IMAGE_LIMIT */
#define CONFIG_SYS_BOOTM_LEN	0x800000	/* use 8MByte as default max gunzip size */
#endif /* MAX_U_IMAGE_LIMIT */
#endif /* CONFIG_SYS_BOOTM_LEN */

bootm_headers_t images;		/* pointers to os/initrd/fdt images */

#ifdef CHECK_HEAD
static int uimage_check_hcrc(const image_header_t *hdr)
{
	ulong hcrc;
	ulong len = sizeof(image_header_t);
	image_header_t header;

	/* Copy header so we can blank CRC field for re-calculation */
	memmove(&header, (char *)hdr, len);
	header.ih_hcrc = cpu_to_uimage(0);

	hcrc = crc32(0, (unsigned char *)&header, len);
	if(hcrc == 0){
		puts("\nhcrc error\n");
		return 0;
	}

	return (hcrc == uimage_to_cpu(hdr->ih_hcrc));
}
#endif

#ifdef CHECK_DATA
static int uimage_check_dcrc(const image_header_t *hdr)
{
	ulong data = (ulong)hdr + sizeof(image_header_t);
	ulong len = uimage_to_cpu(hdr->ih_size);
	ulong dcrc = crc32_wd(0, (unsigned char *)data, len, CHUNKSZ_CRC32);

	return (dcrc == uimage_to_cpu(hdr->ih_dcrc));
}
#endif

static int bootm_find_os(ulong img_addr, ulong size_limit)
{
	image_header_t *hdr = (image_header_t *)img_addr;

#ifdef DEBUG_DATA
	int i;
	printf("hdr = %p\n", hdr);
	for (i = 0; i < 64; i++) {
		if ((i % 4) == 0)
			printf("\n");
		printf("%x ", *((unsigned int *)img_addr + i));
	}
	printf("\n");
#endif

	if (uimage_to_cpu(hdr->ih_magic) != IH_MAGIC){
		printf("uimage_to_cpu is error\n");
		return -1;
	}

#ifdef CHECK_HEAD
	if (!uimage_check_hcrc(hdr)){
		printf("uimage_check_hcrc is error\n");
		return -1;
	}
#endif

#ifdef CHECK_DATA
	if (!uimage_check_dcrc(hdr)){
		printf("uimage_check_hcrc is error\n");
		return -1;
	}
#endif

	images.os.image_start = (ulong)hdr + sizeof(image_header_t);
	images.os.image_len = uimage_to_cpu(hdr->ih_size);
	images.os.load = uimage_to_cpu(hdr->ih_load);
	images.ep = uimage_to_cpu(hdr->ih_ep);

	if (images.os.image_len > size_limit) {
		printf("\nuImage is too large, size = [%d], limit = [%d]\n", images.os.image_len, size_limit);
		return -1;
	}

	printf("\nuImage size = [%d]\n", images.os.image_len);

	return 0;
}

static int bootm_load_os(bootm_headers_t *images)
{
	image_info_t os = images->os;
	ulong load = os.load;
	ulong image_start = os.image_start;
	ulong image_len = os.image_len;
	void *load_buf, *image_buf;
	int ret;

	load_buf = (void *)load;
	image_buf = (void *)image_start;

	/*
	 *load_buf = map_sysmem(load, image_len);
	 *image_buf = map_sysmem(image_start, image_len);
	 */

#if 0
	puts("   Uncompressing ... ");
	if (gunzip(load_buf, CONFIG_SYS_BOOTM_LEN, image_buf, &image_len) != 0) {
		puts("Failed\n");
		return -1;
	}
#endif

	/* puts(" Uncompressing lzma ... "); */
#ifdef CONFIG_LZO
	ret = lzop_decompress(image_buf, image_len, load_buf,
				CONFIG_SYS_BOOTM_LEN);
	if(ret != 0){
		printf("LZO: uncompress or overwrite error%d "
				"- must RESET board to recover\n",ret);
		return -1;

	}
#else
	if(lzmaBuffToBuffDecompress(load_buf, CONFIG_SYS_BOOTM_LEN,
				image_buf, image_len) != 0) {
		puts("LZMA: uncompress or overwrite error "
				"- must RESET board to recover\n");
		return -1;
	}
#endif


	flush_cache(load, image_len * sizeof(ulong));

	puts("OK\n");

	return 0;
}

static void bootm_jump_linux(bootm_headers_t *images)
{
	void (*theKernel)(void);

	printf("images->ep = %x\n", images->ep);

	theKernel = (void (*)(void))images->ep;
	theKernel();
}

int boot_uImage(ulong img_addr, ulong size_limit)
{
	int ret = 0;

	/* find the OS */
	ret = bootm_find_os(img_addr, size_limit);
	if (ret) {
		puts("bootm_find_os error!");
		return -1;
	}

	/* Load the OS */
	ret = bootm_load_os(&images);
	if (ret) {
		puts("bootm_load_os error!");
		return -1;
	}

	/* start the OS */
	bootm_jump_linux(&images);

	return ret;
}
