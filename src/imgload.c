#include "darnit.h"


IMGLOAD_DATA imgloadBAD(const char *fname) {
	IMGLOAD_DATA data;

	data.w = data.h = 0;
	data.img_data = NULL;

	fprintf(stderr, "Unable to load image %s\n", fname);

	return data;
}


IMGLOAD_DATA imgloadSTB(const char *fname) {
	int t;
	IMGLOAD_DATA img;
	
	img.w = img.h = 0;
	img.img_data = (void *) stbi_load(fname, (int *) &img.w, (int *) &img.h, &t, 4);
	
	return img;
}


IMGLOAD_DATA imgloadLoad(const char *fname) {
	if (fname == NULL) return imgloadBAD(fname);
	
	return imgloadSTB(fname);
}


void imgloadDownsample(IMGLOAD_DATA *img, unsigned int target_format) {
	unsigned short *data;
	unsigned short pixel;
	int i;

	if (target_format == PFORMAT_RGBA8)
		return;
	
	data = malloc(sizeof(unsigned short) * img->w * img->h);

	if (target_format == PFORMAT_RGBA4) {
		for (i = 0; i < img->w * img->h; i++) {
			pixel = ((img->img_data[i] & 0xF0) << 8);
			pixel |= ((img->img_data[i] & 0xF000) >> 4);
			pixel |= ((img->img_data[i] & 0xF00000) >> 16);
			pixel |= ((img->img_data[i] & 0xF0000000) >> 28);
			data[i] = pixel;
		}
	} else if (target_format == PFORMAT_RGB5A1) {
		for (i = 0; i < img->w * img->h; i++) {
			pixel = ((img->img_data[i] & 0xF8) << 8);
			pixel |= ((img->img_data[i] & 0xF800) >> 5);
			pixel |= ((img->img_data[i] & 0xF80000) >> 18);
			pixel |= ((img->img_data[i] & 0x80000000) >> 31);
			data[i] = pixel;
		}
	} else
		return;
	
	free(img->img_data);
	img->img_data = (void *) data;

	return;
}
