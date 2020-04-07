#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <jpeglib.h>


#define INPUT_FILE_NAME  "jpg-example.jpg"
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

	jpeg_start_decompress(cinfo);
	row_stride = cinfo->output_width * cinfo->output_components;


	buffer = alloc_img_buf(row_stride, cinfo->output_height);
	//buffer = cinfo->mem->alloc_sarray((j_common_ptr)cinfo, JPOOL_IMAGE, row_stride,
	//cinfo->output_height);
	/*
	 * TODO error handling
	 */
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
	jpeg_set_quality(cinfo, 100, TRUE);

	jpeg_start_compress(cinfo, TRUE);

	while (cinfo->next_scanline < height) {
		jpeg_write_scanlines(cinfo, &img[cinfo->next_scanline], 1);
	}

	jpeg_finish_compress(cinfo);
	jpeg_destroy_compress(cinfo);

	fclose(outfile);

	return 0;
}

int main(int argc, char **argv)
{
	int rc = 0;
	int width, height;
	JSAMPARRAY img = NULL;

	rc = libjpeg_read_file(INPUT_FILE_NAME, &img, &width, &height);

	rc = libjpeg_write_file(OUTPUT_FILE_NAME, img, width, height);
	free_img_buf(img, width, height);
	return rc;
}
