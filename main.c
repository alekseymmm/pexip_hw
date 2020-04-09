#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/param.h>

#include <jpeglib.h>

#include "jpeg_utils.h"
#include "geometry.h"
#include "img_operations.h"

int main(int argc, char **argv)
{
	int rc = 0;
	int width= 0, height = 0;
	int scaled_width = 0, scaled_height = 0;
	JSAMPARRAY img = NULL;
	JSAMPARRAY scaled_img = NULL;
	char *input_filename = NULL;
	char *output_file_name = NULL;

	if (argc != 3) {
		fprintf(stderr,
			"Wrong number of arguments. See usage:\n");
		fprintf(stdout,
			"pexip_hw <input jpg file name> <output file name>\n");
		exit(-EINVAL);
	}
	input_filename = argv[1];
	output_file_name = argv[2];

	rc = libjpeg_read_file(input_filename, &img, &width, &height);
	scaled_width = width / 2;
	scaled_height = height / 2;

	scaled_img = alloc_img_buf(scaled_width * 3, scaled_height);
	img_scale_by_half(img, width, height,
			  scaled_img, scaled_width, scaled_height);

	/* Instead of scaling myself, I can use scaling API from libjpeg-turbo
	 * and scale image while decoding. but I am not sure this is allowed
	 * here. But it works.
	 */
//	rc = libjpeg_read_file_scaled(input_filename, &scaled_img,
//				      &scaled_width, &sscaled_height);
	img_fill_main_triangle(img, width, height,
			       scaled_img, scaled_width, scaled_height);
	//img_crop_lower_triangle(img, width, height);
	img_fill_rotated_triangles(img, width, height);

	rc = libjpeg_write_file(output_file_name, img, width, height);

	free_img_buf(scaled_img, scaled_width, scaled_height);
	free_img_buf(img, width, height);

	fprintf(stdout, "done. See result in %s\n", output_file_name);
	return rc;
}
