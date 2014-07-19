#ifndef _TGA_H
#define _TGA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tex.h"

typedef struct
{
	unsigned char id_length;
	unsigned char color_map_type;
	unsigned char image_type;
	unsigned short color_map_offset;
	unsigned short color_map_entries;
	unsigned char color_map_entry_size;
	unsigned short x_origin;
	unsigned short y_origin;
	unsigned short x_size;
	unsigned short y_size;
	unsigned char pixel_depth;
	unsigned char image_desc;
}
tga_header;

typedef struct
{
	unsigned short extension_size;
	unsigned char author_name[41];
	unsigned char author_comment[324];
	unsigned char data_time[12];
	unsigned char job_id[41];
	unsigned short job_time[3];
	unsigned char software_id[41];
	unsigned char software_version[3];
	unsigned int key_color;
	unsigned int pixel_aspect_ratio;
	unsigned int gamma_value;
	unsigned int color_correction_offset;
	unsigned int postage_stamp_offset;
	unsigned int scan_line_offset;
	unsigned char attr_type;
}
tga_extension;

typedef struct
{
	unsigned int extension_offset;
	unsigned int devel_area_offset;
	unsigned char signature[16];
	unsigned char cdot;
	unsigned char cnull;
}
tga_footer;

void tgaLoadFile(const char* mPath, tex_t** result);

#endif
