#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/param.h>
#include <jpeglib.h>

#include "geometry.h"

int img_rotate(JSAMPARRAY src, int width, int height, float a, JSAMPARRAY dst)
{
	int x_new, y_new;
	int x_old, y_old;
	int x_dif, y_dif;
	int xc = width / 2;
	int yc = height / 2;
	float cosa = cos(a);
	float sina = sin(a);

	for (y_new = 0; y_new < height; y_new++) {
		for (x_new = 0; x_new < width; x_new++) {
			struct point pt, old_pt;

			pt.x = x_new;
			pt.y = y_new;
			rotate_point(&pt, width, height, &old_pt, -a);

			x_old = old_pt.x;
			y_old = old_pt.y;
			if (x_old < 0 || x_old >= width)
				continue;
			if (y_old < 0 || y_old >= height)
				continue;

			dst[y_new][x_new * 3] = src[y_old][x_old * 3];
			dst[y_new][x_new * 3 + 1] = src[y_old][x_old * 3 + 1];
			dst[y_new][x_new * 3 + 2] = src[y_old][x_old * 3 + 2];

		}
	}

	return 0;
}

void img_crop_lower_triangle(JSAMPARRAY img, int width, int height)
{
	int x, y;
	int x_dif;
	struct point pt, v1;
	struct point v2, v3;

	v1.x = width / 2;
	v1.y = height / 2 - 1;

	x_dif = tan(M_PI / 6) * height / 2;

	v2.x = v1.x - x_dif - 1;
	v2.y = height;

	v3.x = v1.x + x_dif + 1;
	v3.y = height;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pt.x = x;
			pt.y = y;
			if (!point_in_triangle(&pt, &v1, &v2, &v3)) {
				float coef = 0.3;

				img[y][x * 3] = img[y][x * 3] * coef;
				img[y][x * 3 + 1] = img[y][x * 3 + 1] * coef;
				img[y][x * 3 + 2] = img[y][x * 3 + 2] * coef;
			}
		}
	}

}

int img_scale_by_half(JSAMPARRAY src, int src_w, int src_h,
		      JSAMPARRAY dst, int dst_w, int dst_h)
{
	int x, y;

	if (dst_w * 2 > src_w ||
	    dst_h * 2 > src_h) {
		fprintf(stderr,
			"Wrong image sizes while scaling by half (%d,%d) and (%d,%d)\n",
			src_w, src_h, dst_w, dst_h);
		return -EINVAL;
	}
	for (y = 0; y < dst_h; y++) {
		int y2 = 2 * y;

		for (x = 0; x < dst_w; x++) {
			int x2 = x * 2;
			int i;
			float p, q;

			for (i = 0; i < 3; i++) {
				p = src[y2][x2 * 3 + i] + src[y2][(x2 + 1) * 3 + i];
				p = p / 2;

				q = src[y2+1][x2 * 3 + i] + src[y2+1][(x2 + 1) * 3 + i];
				q = q / 2;
				dst[y][x*3 + i] = (p + q) / 2;
			}
		}
	}
	return 0;
}

int img_fill_triangle(JSAMPARRAY img, int width, int height, struct point *v,
		      float angle)
{
	int x, y;
	struct point pt;
	int y_min, y_max;
	int x_min, x_max;

	x_min = MIN(v[0].x, v[1].x);
	x_min = MIN(x_min, v[2].x);
	x_max = MAX(v[0].x, v[1].x);
	x_max = MAX(x_max, v[2].x);

	y_min = MIN(v[0].y, v[1].y);
	y_min = MIN(y_min, v[2].y);
	y_max = MAX(v[0].y, v[1].y);
	y_max = MAX(y_max, v[2].y);
	for (y = y_min; y < y_max; y++) {
		for (x = x_min; x < x_max; x++) {
			pt.x = x;
			pt.y = y;
			if (point_in_triangle(&pt, &v[0], &v[1], &v[2])) {
				struct point rot;

				rotate_point(&pt, width, height, &rot, angle);

				if (rot.x < 0 || rot.x > width - 1)
					continue;
				if (rot.y < 0 || rot.y > height - 1)
					continue;
				img[y][x * 3] = img[rot.y][rot.x * 3];
				img[y][x * 3 + 1] = img[rot.y][rot.x * 3 + 1];
				img[y][x * 3 + 2] = img[rot.y][rot.x * 3 + 2];
			}
		}
	}
	return 0;
}

int img_fill_rotated_triangles(JSAMPARRAY img, int width, int height)
{
	int x_dif;
	struct point v[3];

	v[0].x = width / 2;
	v[0].y = height / 2;

	x_dif = tan(M_PI / 6) * height / 2;

	v[1].x = v[0].x - x_dif;
	v[1].y = height;

	v[2].x = v[0].x + x_dif;
	v[2].y = height;

	// This code could be wrapped in a loop over angles
	// initial triangle filled

	//fill first left triangle
	v[1].x = v[0].x - x_dif;
	v[1].y = height;
	rotate_point(&v[1], width, height, &v[2], -M_PI / 3);
	img_fill_triangle(img, width, height, v, M_PI / 3);

	//fill second left triangle
	v[1].x = v[0].x - x_dif;
	v[1].y = 0;
	img_fill_triangle(img, width, height, v, 2 * M_PI / 3);

	v[2].x = v[0].x + x_dif;
	v[2].y = 0;
	img_fill_triangle(img, width, height, v, 3 * M_PI / 3);


	rotate_point(&v[2], width, height, &v[1], -M_PI / 3);
	img_fill_triangle(img, width, height, v, 4 * M_PI / 3);

	v[2].x = v[0].x + x_dif;
	v[2].y = height;
	img_fill_triangle(img, width, height, v, 5 * M_PI / 3);

	return 0;
}

int img_fill_main_triangle(JSAMPARRAY img, int width, int height,
			   JSAMPARRAY scaled_img, int scaled_width,
			   int scaled_height)
{
	int x, y;
	int x_dif;
	struct point pt, v1;
	struct point v2, v3;
	int x_offset = width / 4;
	int y_offset = height / 2;

	v1.x = width / 2;
	v1.y = height / 2 - 2;

	x_dif = tan(M_PI / 6) * height / 2;

	v2.x = v1.x - x_dif - 2;
	v2.y = height;

	v3.x = v1.x + x_dif + 2;
	v3.y = height;

	for (y = 0 ; y < height ; y++) {
		int dy = y - y_offset;

		for (x = 0; x < width; x++) {
			int dx = x - x_offset;

			pt.x = x;
			pt.y = y;
			if (!point_in_triangle(&pt, &v1, &v2, &v3) ||
			    y - y_offset >= scaled_height ||
			    y < y_offset ||
			    x - x_offset >= scaled_width) {
				float coef = 0.3;

				img[y][x * 3] = img[y][x * 3] * coef;
				img[y][x * 3 + 1] = img[y][x * 3 + 1] * coef;
				img[y][x * 3 + 2] = img[y][x * 3 + 2] * coef;
			} else {
				img[y][x * 3] = scaled_img[dy][dx * 3];
				img[y][x * 3 + 1] = scaled_img[dy][dx * 3 + 1];
				img[y][x * 3 + 2] = scaled_img[dy][dx * 3 + 2];
			}
		}
	}
	return 0;
}

/*********************** unused functions *************************************/
void kernel_func(JSAMPARRAY img, int width, int height, int x, int y)
{
	int i;
	int sum;

	if (x < 1 || y < 1 || x + 1 == width || y + 1 == height)
		return;
	for (i = 0; i < 3; i++) {
		sum = img[y-1][(x-1)*3+i]+
		      img[y-1][x*3+i]+
		      img[y-1][(x+1)*3+i]+
		      img[y][(x-1)*3+i]+
		      img[y][x*3+i]+
		      img[y][(x+1)*3]+
		      img[y+1][(x-1)*3+i]+
		      img[y+1][x*3+i]+
		      img[y+1][(x+1)*3+i];
		img[y][x*3+i] = sum / 9;
	}
}

int smooth_line(JSAMPARRAY img, int width, int height,
		struct point *v0, struct point *v1)
{
	float k = 0, b = 0;
	int x, y;
	int y_line;

	if (v1->x - v0->x != 0) {
		k = (float)(v1->y - v0->y) / (v1->x - v0->x);
		b = v0->y-k*v0->x;
	} else {
		k = 0;
		b = v0->y;
	}

	for (y = MIN(v0->y, v1->y); y < MAX(v0->y, v1->y); y++) {
		for (x = MIN(v0->x, v1->x); x < MAX(v0->x, v1->x); x++) {
			y_line = k * x + b;
			if (abs(y_line - y) < 2)
				kernel_func(img, width, height, x, y);
		}
	}
	return 0;
}
