/*
 * jpeg_utils.h
 *
 *  Created on: Apr 8, 2020
 *      Author: alex
 */

#ifndef JPEG_UTILS_H_
#define JPEG_UTILS_H_

JSAMPARRAY alloc_img_buf(int w, int h);
void free_img_buf(JSAMPARRAY img, int w, int h);

int libjpeg_read_file(char *file_name, JSAMPARRAY *img,
		      int *width, int *height);
int libjpeg_read_file_scaled(char *file_name, JSAMPARRAY *img,
			     int *width, int *height);
int libjpeg_write_file(char *file_name, JSAMPARRAY img, int width, int height);

#endif /* JPEG_UTILS_H_ */
