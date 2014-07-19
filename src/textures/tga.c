#include "tga.h"

void tgaLoadFile(const char* mPath, tex_t** result)
{
	FILE *f = fopen(mPath, "rb");
	
	printf("TGA Reader: Reading %s file\n", mPath);
	
	tga_header *header = (tga_header*)malloc(sizeof(tga_header));
	fread(&(header->id_length), sizeof(unsigned char), 1, f);
	fread(&(header->color_map_type), sizeof(unsigned char), 1, f);
	fread(&(header->image_type), sizeof(unsigned char), 1, f);
	fread(&(header->color_map_offset), sizeof(unsigned short), 1, f);
	fread(&(header->color_map_entries), sizeof(unsigned short), 1, f);
	fread(&(header->color_map_entry_size), sizeof(unsigned char), 1, f);
	fread(&(header->x_origin), sizeof(unsigned short), 1, f);
	fread(&(header->y_origin), sizeof(unsigned short), 1, f);
	fread(&(header->x_size), sizeof(unsigned short), 1, f);
	fread(&(header->y_size), sizeof(unsigned short), 1, f);
	fread(&(header->pixel_depth), sizeof(unsigned char), 1, f);
	fread(&(header->image_desc), sizeof(unsigned char), 1, f);
	
	unsigned char* data;
	
	switch(header->image_type)
	{
		case 0:
			break;
		
		case 1:
			/* TODO: Uncompressed color-mapped image */
			break;
		
		case 2:
			/* Uncompressed true-color image */
			{
				data = (unsigned char*)malloc(sizeof(unsigned char) * header->x_size * header->y_size * 4);
				
				for(int i = 0; i < header->y_size; i++)
				{
					for(int j = 0; j < header->x_size; j++)
					{
						unsigned char colors[4];
						fread(colors, sizeof(unsigned char) * header->pixel_depth / 8, 1, f);
						
						data[(j + i * header->x_size) * 4 + 0] = colors[2];
						data[(j + i * header->x_size) * 4 + 1] = colors[1];
						data[(j + i * header->x_size) * 4 + 2] = colors[0];
						data[(j + i * header->x_size) * 4 + 3] = header->pixel_depth == 32 ? colors[3] : 255;
					}
				}
			}
			break;
		
		case 3:
			/* TODO: Uncompressed B&W image */
			break;
		
		case 9:
			/* TODO: RLE color-mapped image */
			break;
		
		case 10:
			/* RLE true-color image */
			{
				unsigned char rle_id = 0;
				int i = 0;
				int read = 0;
			
				data = (unsigned char*)malloc(sizeof(unsigned char) * header->x_size * header->y_size * 4);
				
				while(i < header->x_size * header->y_size)
				{
					fread(&rle_id, sizeof(unsigned char), 1, f);
					
					if(rle_id < 128)
					{
						rle_id++;
						
						while(rle_id)
						{
							unsigned char colors[4];
							fread(colors, sizeof(unsigned char) * header->pixel_depth / 8, 1, f);
						
							data[read + 0] = colors[2];
							data[read + 1] = colors[1];
							data[read + 2] = colors[0];
							data[read + 3] = header->pixel_depth == 32 ? colors[3] : 255;
							
							i++;
							rle_id--;
							read += 4;
						}
					}
					else
					{
						rle_id -= 127;
						
						unsigned char colors[4];
						fread(colors, sizeof(unsigned char) * header->pixel_depth / 8, 1, f);
						
						while(rle_id)
						{
							data[read + 0] = colors[2];
							data[read + 1] = colors[1];
							data[read + 2] = colors[0];
							data[read + 3] = header->pixel_depth == 32 ? colors[3] : 255;
							
							i++;
							rle_id--;
							read += 4;
						}
					}
				}
			}
			break;
		
		case 11:
			/* TODO: RLE B&W image */
			break;
			
		default:
			break;
	}
	
	fclose(f);
	
	(*result) = (tex_t*)malloc(sizeof(tex_t));
	(*result)->w = header->x_size;
	(*result)->h = header->y_size;
	(*result)->data = (unsigned char*)malloc(sizeof(unsigned char) * (*result)->w * (*result)->h * 4);
	
	for(unsigned int i = 0; i < (*result)->w * (*result)->h * 4; i++)
	{
		(*result)->data[i] = data[i];
	}
	
	free(header);
	header = NULL;
	
	free(data);
	data = NULL;
}
