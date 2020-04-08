#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/param.h>

#include <jpeglib.h>


#define INPUT_FILE_NAME  "jpg-example.jpg"
//##define INPUT_FILE_NAME  "unnamed.jpg"
#define OUTPUT_FILE_NAME  "out.jpg"

JSAMPARRAY alloc_img_buf(int w, int h)
{
	int i;
	char **ptrs;

	ptrs = malloc((sizeof(JSAMPROW)) * h);
	for (i = 0; i < h; i++) {
		ptrs[i] = calloc(w, 1);
	}
	return (JSAMPARRAY)ptrs;
}

void free_img_buf(JSAMPARRAY img, int w, int h)
{
	int i;
	for (i = 0; i < h; i++) {
		free(img[i]);
	}
	free(img);
}

int libjpeg_read_file(char *file_name, JSAMPARRAY *img, int *width, int *height)
{
	FILE *infile;
	struct jpeg_decompress_struct decompress_info;
	struct jpeg_decompress_struct *cinfo = &decompress_info;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	int i;

	infile = fopen(file_name, "rb");
	if (!infile){
		fprintf(stderr, "can not open file: %s\n", INPUT_FILE_NAME);
		return -ENOENT;
	}




	cinfo->err = jpeg_std_error(&jerr);
	jpeg_create_decompress(cinfo);

	jpeg_stdio_src(cinfo, infile);
	jpeg_read_header(cinfo, TRUE);
//	/cinfo->scale_num = 1;
	//cinfo->scale_denom = 2;

	jpeg_start_decompress(cinfo);
	row_stride = cinfo->output_width * cinfo->output_components;


	buffer = alloc_img_buf(row_stride, cinfo->output_height);
	i = 0;
	while (cinfo->output_scanline < cinfo->output_height) {
		jpeg_read_scanlines(cinfo, &buffer[i], 1);
		i++;
	}

	*img = buffer;
	*width = cinfo->output_width;
	*height = cinfo->output_height;

	jpeg_finish_decompress(cinfo);
	jpeg_destroy_decompress(cinfo);

	fclose(infile);

	return 0;
}


int libjpeg_read_file_scaled(char *file_name, JSAMPARRAY img, int *width, int *height)
{
	FILE *infile;
	struct jpeg_decompress_struct decompress_info;
	struct jpeg_decompress_struct *cinfo = &decompress_info;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	int i;
	int y_offset, x_offset;

	infile = fopen(file_name, "rb");
	if (!infile){
		fprintf(stderr, "can not open file: %s\n", INPUT_FILE_NAME);
		return -ENOENT;
	}

	cinfo->err = jpeg_std_error(&jerr);
	jpeg_create_decompress(cinfo);

	jpeg_stdio_src(cinfo, infile);
	jpeg_read_header(cinfo, TRUE);

	cinfo->scale_num = 1;
	cinfo->scale_denom = 2;

	jpeg_start_decompress(cinfo);
	row_stride = cinfo->output_width * cinfo->output_components;


	i = 0;
	y_offset = cinfo->output_height - 1;
	x_offset = cinfo->output_width / 2;
	while (cinfo->output_scanline < cinfo->output_height) {
		jpeg_read_scanlines(cinfo, &img[i], 1);
		i++;
	}

	*width = cinfo->output_width;
	*height = cinfo->output_height;


	jpeg_finish_decompress(cinfo);
	jpeg_destroy_decompress(cinfo);

	fclose(infile);

	return 0;
}

int libjpeg_write_file(char *file_name, JSAMPARRAY img, int width, int height)
{
	FILE *outfile;
	struct jpeg_compress_struct compress_info;
	struct jpeg_compress_struct *cinfo = &compress_info;
	struct jpeg_error_mgr jerr;
	int i;

	outfile = fopen(file_name, "wb");
	if (!outfile){
		fprintf(stderr, "can not open file: %s\n", INPUT_FILE_NAME);
		return -ENOENT;
	}

	cinfo->err = jpeg_std_error(&jerr);
	jpeg_create_compress(cinfo);

	jpeg_stdio_dest(cinfo, outfile);
	cinfo->image_width = width;
	cinfo->image_height = height;
	cinfo->input_components = 3;
	cinfo->in_color_space = JCS_RGB;


	jpeg_set_defaults(cinfo);
	//jpeg_set_quality(cinfo, 100, TRUE);

	jpeg_start_compress(cinfo, TRUE);

	while (cinfo->next_scanline < height) {
		jpeg_write_scanlines(cinfo, &img[cinfo->next_scanline], 1);
	}

	jpeg_finish_compress(cinfo);
	jpeg_destroy_compress(cinfo);

	fclose(outfile);

	return 0;
}


int rotate_img(JSAMPARRAY src, int width, int height, float a, JSAMPARRAY dst)
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
			x_dif = x_new - xc;
			//y_dif = y_new - yc;
			y_dif = yc - y_new ;

			x_old = xc + (x_dif * cosa + y_dif * sina);
			y_old = yc - (- x_dif * sina + y_dif * cosa);
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

struct point {
	long x;
	long y;
};

long sign (struct point *p1, struct point *p2, struct point *p3)
{
    return (p1->x - p3->x) * (p2->y - p3->y) - (p2->x - p3->x) * (p1->y - p3->y);
}

int point_in_triangle(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3)
{
    float d1, d2, d3;
    int has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

int point_in_triangle2(struct point *pt, struct point *v1, struct point *v2,
		      struct point *v3)
{
	float area;
	float s, t;
	area = 0.5 *(-v2->y*v3->x + v1->y*(-v2->x + v3->x) + v1->x*(v2->y - v3->y) + v2->x*v3->y);
	s = 1/(2*area)*(v1->y*v3->x - v1->x*v3->y + (v3->y - v1->y)*pt->x + (v1->x - v3->x)*pt->y);
	t = 1/(2*area)*(v1->x*v2->y - v1->y*v2->x + (v1->y - v2->y)*pt->x + (v2->x - v1->x)*pt->y);

	return (s > 0 && t > 0 && (1-s-t > 0));
}
#if 0
bool intpoint_inside_trigon(intPoint s, intPoint a, intPoint b, intPoint c)
{
    int as_x = s.x-a.x;
    int as_y = s.y-a.y;

    bool s_ab = (b.x-a.x)*as_y-(b.y-a.y)*as_x > 0;

    if((c.x-a.x)*as_y-(c.y-a.y)*as_x > 0 == s_ab) return false;

    if((c.x-b.x)*(s.y-b.y)-(c.y-b.y)*(s.x-b.x) > 0 != s_ab) return false;

    return true;
}
#endif
void crop_img_lower_triangle(JSAMPARRAY img, int width, int height)
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
				/*
				img[y][x * 3] = 0;
				img[y][x * 3 + 1] = 0;
				img[y][x * 3 + 2] = 0;
				*/
				float coef = 0.3;
				img[y][x * 3] = img[y][x * 3] * coef;
				img[y][x * 3 + 1] = img[y][x * 3 + 1] * coef;
				img[y][x * 3 + 2] = img[y][x * 3 + 2] * coef;
			}
		}
	}

}

void rotate_point(struct point *pt, int width, int height, struct point *res,
			bool counterclockwise)
{
	int x_new, y_new;
	float x_dif, y_dif;
	int xc = width / 2;
	int yc = height / 2;

	float a =  M_PI / 3;
	float cosa = cos(a);
	float sina = sin(a);

	x_dif = pt->x - xc;
	//y_dif = pt->y - yc;
	y_dif = yc - pt->y;
	if (counterclockwise) {
		x_new = xc + (x_dif * cosa - y_dif * sina);
		y_new = yc - (x_dif * sina + y_dif * cosa);
	} else {
		x_new = xc + (x_dif * cosa + y_dif * sina);
		y_new = yc - (-x_dif * sina + y_dif * cosa);
	}
	res->x = x_new;
	res->y = y_new;
}

void rotate_point2(struct point *pt, int width, int height, struct point *res,
		   float a)
{
	int x_new, y_new;
	float x_dif, y_dif;
	int xc = width / 2;
	int yc = height / 2;

	//float a =  M_PI / 3;
	float cosa = cos(a);
	float sina = sin(a);

	x_dif = pt->x - xc;
	//y_dif = pt->y - yc;
	y_dif = yc - pt->y;

		x_new = xc + (x_dif * cosa - y_dif * sina);
		y_new = yc - (x_dif * sina + y_dif * cosa);

	res->x = x_new;
	res->y = y_new;
}

int fill_triangle(JSAMPARRAY img, int width, int height, struct point *v,
		  bool counterclockwise)
{
	int x, y;
	struct point pt;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pt.x = x;
			pt.y = y;
			if (point_in_triangle(&pt, &v[0], &v[1], &v[2])) {
				struct point rot;

				rotate_point(&pt, width, height, &rot, counterclockwise);

				if (rot.x < 0 || rot.x > width - 1)
					continue;
				if (rot.y < 0 || rot.y > height -1)
					continue;
				img[y][x * 3] = img[rot.y][rot.x *3];
				img[y][x * 3 + 1] = img[rot.y][rot.x *3 + 1];
				img[y][x * 3 + 2] = img[rot.y][rot.x *3 + 2];
			}
		}
	}
}


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
		img[y][x*3 +i] = sum / 9;
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
		b = v0->y - k* v0->x;
	} else {
		k = 0;
		b= v0->y;
	}

	for (y = MIN(v0->y, v1->y); y < MAX(v0->y, v1->y); y++){
		for (x = MIN(v0->x, v1->x); x < MAX(v0->x, v1->x); x++) {
			y_line = k * x + b;
			if (abs(y_line - y) < 2)
				kernel_func(img, width, height, x, y);
		}
	}

}

int fill_triangle2(JSAMPARRAY img, int width, int height, struct point *v,
		   float angle)
{
	int x, y;
	struct point pt;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			pt.x = x;
			pt.y = y;
			if (point_in_triangle(&pt, &v[0], &v[1], &v[2])) {
				struct point rot;

				rotate_point2(&pt, width, height, &rot, angle);

				if (rot.x < 0 || rot.x > width - 1)
					continue;
				if (rot.y < 0 || rot.y > height -1)
					continue;
				img[y][x * 3] = img[rot.y][rot.x *3];
				img[y][x * 3 + 1] = img[rot.y][rot.x *3 + 1];
				img[y][x * 3 + 2] = img[rot.y][rot.x *3 + 2];
			}
		}
	}
}


int fill_rotated_triangles(JSAMPARRAY img, int width, int height)
{
	int x, y;
	int x_dif, y_dif;
	struct point pt;
	struct point v[3];
	struct point new_v;

	int i;

	v[0].x = width / 2 ;
	v[0].y = height / 2 ;

	x_dif = tan(M_PI / 6) * height / 2;

	v[1].x = v[0].x - x_dif;
	v[1].y = height;

	v[2].x = v[0].x + x_dif ;
	v[2].y = height;
// initial triangle filled
	//fill first right triangle

//	rotate_point(&v[2], width, height, &new_v, true);
//
//	v[1] = v[2];
//	v[2] = new_v;
//	fill_triangle2(img, width, height, v, false, M_PI / 3);


//	fill_triangle(img, width, height, v, false);

//	//fill first left triangle
	v[1].x = v[0].x - x_dif;
	v[1].y = height;
	rotate_point2(&v[1], width, height, &v[2], -M_PI / 3);;
	fill_triangle2(img, width, height, v, M_PI / 3);

	//fill second left triangle
	v[1].x = v[0].x - x_dif;
	v[1].y = 0;
	fill_triangle2(img, width, height, v, 2 * M_PI / 3);

	v[2].x = v[0].x + x_dif;
	v[2].y = 0;
	fill_triangle2(img, width, height, v, 3 * M_PI / 3 );


	rotate_point2(&v[2], width, height, &v[1], -M_PI / 3);
	fill_triangle2(img, width, height, v, 4 * M_PI / 3);

	v[2].x = v[0].x + x_dif;
	v[2].y = height;
	fill_triangle2(img, width, height, v, 5 * M_PI / 3);

#if 0
	for (i = 0; i < 4; i++) {
//		rotate_point(&v[0], width, height, &new_v, true);
//		v[0] = new_v;
//
//		rotate_point(&v[1], width, height, &new_v, true);
//		v[1] = new_v;

		rotate_point(&v[2], width, height, &new_v, true);
//		v[2] = new_v;
		v[1] = v[2];
		v[2] = new_v;

		fill_triangle(img, width, height, v);
//		smooth_line(img, width, height, &v[0], &v[1]);
	}
#endif
}

int img_scale_by_half(JSAMPARRAY src, int src_w, int src_h,
		      JSAMPARRAY dst, int tgt_w, int tgt_h)
{
	int x, y;

	for (y = 0; y < tgt_h; y++) {
		int y2 = 2 * y;
		for (x = 0; x < tgt_w; x++) {
			int x2 = x * 2;
			int i;
			float p, q;

			for (i = 0; i < 3; i++) {
				p = src[y2][x2 * 3 + i] + src[y2][(x2+1)*3 + i];
				p = p / 2;

				q = src[y2+1][x2 * 3 + i] + src[y2+1][(x2+1)*3 + i];
				q = q / 2;
				dst[y][x*3 + i] = (p + q) / 2;
			}
		}
	}
}

int fill_main_triangle(JSAMPARRAY img, int width, int height,
		       JSAMPARRAY scaled_img, int scaled_width, int scaled_height)
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
		for (x = 0; x < width; x++) {
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
				img[y][x * 3] = scaled_img[y - y_offset][(x- x_offset)*3];
				img[y][x * 3+ 1] = scaled_img[y - y_offset][(x- x_offset)*3 + 1];
				img[y][x * 3+ 2] = scaled_img[y - y_offset][(x- x_offset)*3 + 2];
			}
		}
	}

}


int main(int argc, char **argv)
{
	int rc = 0;
	int width, height;
	int scaled_width, scaled_height;
	JSAMPARRAY img = NULL;
	JSAMPARRAY scaled_img = NULL;
	char *input_filename;
	char *output_file_name;

	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments. See usage:\n");
		fprintf(stdout, "pexip_hw <input jpg file name> <output file name>\n");
		exit(-EINVAL);
	}
	input_filename = argv[1];
	output_file_name = argv[2];

	rc = libjpeg_read_file(input_filename, &img, &width, &height);
	scaled_width = width / 2;
	scaled_height = height / 2;
	scaled_img = alloc_img_buf(scaled_width * 3, scaled_height);

	img_scale_by_half(img, width, height, scaled_img, scaled_width, scaled_height);

//	rc = libjpeg_read_file_scaled(INPUT_FILE_NAME, small_img, &small_width,
//			 	      &small_height);
	fill_main_triangle(img, width, height, scaled_img, scaled_width, scaled_height);
	//crop_img_lower_triangle(img, width, height);
	fill_rotated_triangles(img, width, height);

	rc = libjpeg_write_file(output_file_name, img, width, height);

	free_img_buf(scaled_img, scaled_width, scaled_height);
	free_img_buf(img, width, height);

	printf("done. See result in %s\n", output_file_name);
	return rc;
}
