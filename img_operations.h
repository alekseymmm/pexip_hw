#ifndef IMG_OPERATIONS_H_
#define IMG_OPERATIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/param.h>
#include <jpeglib.h>

#include "geometry.h"

int img_rotate(JSAMPARRAY src, int width, int height, float a, JSAMPARRAY dst);
void img_crop_lower_triangle(JSAMPARRAY img, int width, int height);
int img_scale_by_half(JSAMPARRAY src, int src_w, int src_h,
		      JSAMPARRAY dst, int dst_w, int dst_h);

int img_fill_triangle(JSAMPARRAY img, int width, int height, struct point *v,
		      float angle);
int img_fill_rotated_triangles(JSAMPARRAY img, int width, int height);
int img_fill_main_triangle(JSAMPARRAY img, int width, int height,
		           JSAMPARRAY scaled_img, int scaled_width,
			   int scaled_height);

#endif /* IMG_OPERATIONS_H_ */
