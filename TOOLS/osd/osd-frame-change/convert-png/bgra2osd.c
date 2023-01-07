/*
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char **argv)
{

	FILE *ifp;
	FILE *ofp;
	char *name = NULL;
	char outfile[256];
	int width = 0;
	int height = 0;
	int size = 0;
	unsigned char data[2048];
	int len, i;

	if(argc < 4){
		printf("Invalid parameters!\n");
		printf("The format : ./bgra2osd infile width height\n");
		printf("For example: ./bgra2osd aa.brga 100 80\n");
		printf("The name of output file is logodata_100x80_osd.h\n");
		return 0;
	}
	
	name = argv[1];
	width = atoi(argv[2]);
	height = atoi(argv[3]);
	printf("%s\n", name);

	/* check input file */
	if (NULL == (ifp = fopen(name, "r"))) {
		printf("%s fopen err\n", name);
		return 0;
	}
	
	fseek(ifp, 0, SEEK_END);
	size = ftell(ifp);
	rewind(ifp);

	if(size != width * height * 4){
		printf("The size of %s is wrong; it must be width*height*4\n", name);
		
	printf("[INFO] size = %d width * height * 4 =  %d \n",size,width * height * 4);
		fclose(ifp);
		return 0;
	}

	
	/* open output file */
	memset(outfile, 0, sizeof(outfile));
	sprintf(outfile, "logodata_%dx%d_bgra.h", width, height);
	if (NULL == (ofp = fopen(outfile, "w"))) {
		printf("%s fopen err\n", outfile);
		fclose(ifp);
		return 0;
	}

	fprintf(ofp,
			"/*\n"
			" *  %s\n"
			" *\n"
			" */\n"
			"#ifndef __LOGODATA_INFO_OSD_H__\n"
			"#define __LOGODATA_INFO_OSD_H__\n",
			outfile);
	fprintf(ofp, "\n");

	fprintf(ofp,
			"#ifdef __cplusplus\n"
			"#if __cplusplus\n"
			"extern %cC%c\n"
			"{\n"
			"#endif\n"
			"#endif /* __cplusplus */\n",
			0x22, 0x22);

	fprintf(ofp, "uint8_t logodata_%dx%d_bgra[%d] = {\n", width, height, size);

	while((len = fread(data, 1, sizeof(data), ifp))){
		for(i = 0; i < len; i++){
			fprintf(ofp, "0x%02x,", data[i]);
			if(i % 32 == 0)
				fprintf(ofp, "\n");
		}
		if(len < sizeof(data))
			break;
	}

	fprintf(ofp, "};\n");
	fprintf(ofp, "\n");
	fprintf(ofp,
			"#ifdef __cplusplus\n"
			"#if __cplusplus\n"
			"}\n"
			"#endif\n"
			"#endif /* __cplusplus */\n");
	fprintf(ofp, "#endif /* __LOGODATA_INFO_OSD_H__ */\n");
	fclose(ofp);
	fclose(ifp);
}
