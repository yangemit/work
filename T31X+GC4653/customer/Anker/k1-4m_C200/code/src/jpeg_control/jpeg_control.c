#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <jpeg_control.h>
#include <jconfig.h>
#include <jpeglib.h>
#include <setjmp.h>

static jmp_buf setjmp_buffer;
METHODDEF(void) jpeg_mem_error_exit (j_common_ptr cinfo)
{
	longjmp(setjmp_buffer, 1);
}

unsigned char *read_jpeg_data(unsigned char*jpeg_data, int jpeg_size, int *width, int *height)
{

	if (!jpeg_data) {
		printf("ERROR(%s): jpeg data error!!!\n", __func__);
		return NULL;
	}

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	/*Step1: set jpeg callback */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_mem_error_exit;
	if (setjmp(setjmp_buffer)) {
		printf("ERROR(%s): jpeg decode error!!!\n", __func__);
		return NULL;
	}

	jpeg_create_decompress(&cinfo);
	/* jpeg_stdio_src(&cinfo, fp); */
	jpeg_mem_src(&cinfo, (const unsigned char*)jpeg_data, jpeg_size);

	int ret = jpeg_read_header(&cinfo, TRUE);
	if(ret != JPEG_HEADER_OK) {
		printf("ERROR(%s): jpeg_read_header failed\n", __func__);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	/*Step2: start decompress */
	bool bStart = jpeg_start_decompress(&cinfo);
	if(!bStart){
		printf("ERROR(%s): jpeg_start_decompress failed\n", __func__);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	unsigned int w = cinfo.output_width;
	unsigned int h = cinfo.output_height;
	int numChannels = cinfo.num_components; // 3 = RGB, 4 = RGBA
	unsigned long dataSize = w * h * numChannels;

	/* printf("width = %d, height = %d, dataSize = %ld, numChannels = %d\n", w, h, dataSize, numChannels); */

	// read RGB(A) scanlines one at a time into jdata[]
	unsigned char *data = (unsigned char *)malloc(dataSize);
	if(!data) {
		printf("ERROR(%s): malloc data error failed\n", __func__);
		return NULL;
	}

	unsigned char* rowptr;
	while (cinfo.output_scanline < h)
	{
		rowptr = data + cinfo.output_scanline * w * numChannels;
		jpeg_read_scanlines( &cinfo, &rowptr, 1 );
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	*width = w;
	*height = h;

	return data;
}

unsigned char *write_jpeg_data (unsigned char*jpeg_data, int jpeg_size, unsigned char* rgb_buffer, int quality, int numChannels, int image_width, int image_height, unsigned long *image_size)
{

	if(!jpeg_data || !rgb_buffer) {
		printf("ERROR(%s): input param invlaid !!!\n", __func__);
		return NULL;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	unsigned char *out_buffer = (unsigned char *)malloc(image_width * image_height * numChannels);
	if (!out_buffer) {
		printf("ERROR(%s): malloc data error!!!\n", __func__);
		return NULL;
	}

	/* FILE * outfile;              /1* target file *1/ */
	JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
	int row_stride;             /* physical row width in image buffer */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_mem_error_exit;
	if (setjmp(setjmp_buffer)) {
		printf("ERROR(%s): jpeg decode error!!!\n", __func__);
		return NULL;
	}

	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);
	/* jpeg_stdio_dest(&cinfo, outfile); */
	jpeg_mem_dest(&cinfo,&out_buffer, image_size);

	cinfo.image_width = image_width;               /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = numChannels;          /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;                /* colorspace of input image */

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);      /* limit to baseline-JPEG values */

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = image_width * numChannels;       /* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & rgb_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	return out_buffer;
}

static void StretchColors(void* pDest, int nDestWidth, int nDestHeight, int nDestBits, void* pSrc, int nSrcWidth, int nSrcHeight, int nSrcBits)
{
	//参数有效性检查
	//ASSERT_EXP(pDest != NULL);
	//ASSERT_EXP((nDestBits == 32) || (nDestBits == 24));
	//ASSERT_EXP((nDestWidth > 0) && (nDestHeight > 0));

	//ASSERT_EXP(pSrc != NULL);
	//ASSERT_EXP((nSrcBits == 32) || (nSrcBits == 24));
	//ASSERT_EXP((nSrcWidth > 0) && (nSrcHeight > 0));

	//令dfAmplificationX和dfAmplificationY分别存储水平和垂直方向的放大率
	double dfAmplificationX = ((double)nDestWidth)/nSrcWidth;
	double dfAmplificationY = ((double)nDestHeight)/nSrcHeight;

	//计算单个源位图颜色和目的位图颜色所占字节数
	const int nSrcColorLen = nSrcBits/8;
	const int nDestColorLen = nDestBits/8;

	//进行图片缩放计算
	for(int i = 0; i<nDestHeight; i++)      //处理第i行
		for(int j = 0; j<nDestWidth; j++)   //处理第i行中的j列
		{
			//------------------------------------------------------
			//以下代码将计算nLine和nRow的值,并把目的矩阵中的(i, j)点
			//映射为源矩阵中的(nLine, nRow)点,其中,nLine的取值范围为
			//[0, nSrcHeight-1],nRow的取值范围为[0, nSrcWidth-1],

			double tmp = i/dfAmplificationY;
			int nLine = (int)tmp;

			if(tmp - nLine > 0.5)
				++nLine;

			if(nLine >= nSrcHeight)
				--nLine;

			tmp = j/dfAmplificationX;
			int nRow = (int)tmp;

			if(tmp - nRow > 0.5)
				++nRow;

			if(nRow >= nSrcWidth)
				--nRow;

			unsigned char *pSrcPos = (unsigned char*)pSrc + (nLine*nSrcWidth + nRow)*nSrcColorLen;
			unsigned char *pDestPos = (unsigned char*)pDest + (i*nDestWidth + j)*nDestColorLen;

			//把pSrcPos位置的前三字节拷贝到pDestPos区域
			*pDestPos++ = *pSrcPos++;
			*pDestPos++ = *pSrcPos++;
			*pDestPos++ = *pSrcPos++;

			if(nDestColorLen == 4)
				*pDestPos = 0xff;
		}
}

unsigned char *resize_jpeg_file(unsigned char *jpeg_data, int jpeg_size, int dest_w, int dest_h, int dest_bits, unsigned long *dest_size)
{
	int src_w = 0, src_h = 0;
	int src_bits = dest_bits;
	int numChannels = dest_bits / 8;
	unsigned char *rgb_data = NULL;
	unsigned char *rgb2_data = NULL;
	unsigned char *out_data = NULL;

	/*Step1: read jpeg data */
	rgb_data = read_jpeg_data(jpeg_data, jpeg_size, &src_w, &src_h);
	if (!rgb_data) {
		printf("ERROR(%s): read_jpeg_file error\n", __func__);
		return NULL;
	}

	rgb2_data = malloc(dest_w * dest_h * dest_bits / 8);
	if (!rgb2_data) {
		printf("ERROR(%s): malloc rgb2_data  error\n", __func__);
		free(rgb_data);
		return NULL;
	}

	/*Step2: resize_rgb_data */
	StretchColors(rgb2_data, dest_w, dest_h, dest_bits, rgb_data, src_w, src_h, src_bits);

	/*Step3: save_jpeg_file */
	out_data = write_jpeg_data(jpeg_data, jpeg_size, rgb2_data, JPEG_QUALITY, numChannels, dest_w, dest_h, dest_size);

	free(rgb_data);
	free(rgb2_data);

	return out_data;
}

unsigned char* convert_jpeg_to_nv12(unsigned char *jpeg_data, int jpeg_size, unsigned int *nv12_w, unsigned int *nv12_h)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride;
	unsigned char *row_line = NULL;

	if (!jpeg_data) {
		printf("ERROR(%s): jpeg data error!!!\n", __func__);
		return NULL;
	}

	/*Step1: aet jpeg callback */
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = jpeg_mem_error_exit;
	if (setjmp(setjmp_buffer)) {
		printf("ERROR(%s): jpeg decode error!!!\n", __func__);
		return NULL;
	}

	/*Step2: jpeg init*/
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, (const unsigned char*)jpeg_data, jpeg_size);
	jpeg_read_header(&cinfo, TRUE);
        if((cinfo.image_width % 2) == 1)
		cinfo.image_width -= 1;
	if((cinfo.image_height % 2) == 1)
		cinfo.image_height -= 1;

	*nv12_w = cinfo.image_width;
	*nv12_h = cinfo.image_height;
	/* printf("image width %d, height %d\n", cinfo.image_width, cinfo.image_height); */
	cinfo.scale_num = 1;
	cinfo.scale_denom = 1;
	cinfo.out_color_space = JCS_YCbCr;
	jpeg_start_decompress(&cinfo);
        if((cinfo.output_width % 2) == 1)
                cinfo.output_width -= 1;
        if((cinfo.output_height % 2) == 1)
                cinfo.output_height -= 1;
	/*printf("output width %d, height %d\n", cinfo.output_width, cinfo.output_height);*/
	row_stride = cinfo.output_width * cinfo.output_components;

	/*Step3: malloc buffer */
	unsigned char *nv12_data = NULL;
	nv12_data = (unsigned char*)malloc(cinfo.image_width * cinfo.image_height * 3 / 2);
	if(!nv12_data){
		printf("(%s,%d): malloc error!\n", __func__, __LINE__);
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	memset(nv12_data, 0x0, cinfo.image_width * cinfo.image_height * 3 / 2);
	unsigned char *nv12_y_buf  = (unsigned char*)nv12_data;
	unsigned char *nv12_uv_buf = (unsigned char*)nv12_data + cinfo.output_width*cinfo.output_height;

	row_line = (unsigned char *)malloc(cinfo.output_width * 3);
	if (!row_line) {
		printf("ERROR(%s): malloc row_line failed!\n", __func__);
		jpeg_destroy_decompress(&cinfo);
		free(nv12_data);
		return NULL;
	}
	memset(row_line, 0x0, cinfo.output_width * 3);

	/* copy jpeg data to yuv */
	int j = 0, k = 0, pair = 0, pix_uv_oft = 0;
	int i = 0;
	while (cinfo.output_scanline < cinfo.output_height)
	{
		(void) jpeg_read_scanlines(&cinfo, &row_line, 1);
		pair = row_stride / 3;

		for (j = 0; j < pair; j++) {
			nv12_y_buf[j + i * cinfo.output_width] = row_line[j*3];
			if ((i % 2) == 0) {
				if ((k % 2) == 0) {
					nv12_uv_buf[j + pix_uv_oft]  = row_line[j*3 + 1];
					nv12_uv_buf[j + 1 + pix_uv_oft]  = row_line[j*3 + 2];
				}
				k++;
			}
		}

		if ((i % 2) == 0) {
			pix_uv_oft += cinfo.output_width;
		}

		i++;
	}
	free(row_line);
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return nv12_data;
}
