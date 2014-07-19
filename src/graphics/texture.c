#include "texture.h"
#include "error.h"
#include "../memory/memory.h"

#define TEXTURE_BASE_COUNT 64
#define TEXTURE_UNITS 8

struct texture_t* textures = NULL;
unsigned int texture_count = TEXTURE_BASE_COUNT + 1;
unsigned int texture_used = 0;
unsigned int texture_active[TEXTURE_UNITS] = {0};
unsigned int texel_active = 0;

void gfxGenTexture(unsigned int* id)
{
	if(textures == NULL)
	{
		textures = malloc(sizeof(struct texture_t) * texture_count);
		
		for(int i = 0; i < texture_count; i++)
		{
			textures[i].data = NULL;
		}
		
		if(!textures)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for texture.\n");
		}
	}

	texture_used++;
	
	if(texture_used == texture_count)
	{
		texture_count *= 2;
		texture_count += 1;
		
		textures = realloc(textures, sizeof(struct texture_t) * texture_count);
		
		for(int i = 0; i < texture_count; i++)
		{
			textures[i].data = NULL;
		}
		
		if(!textures)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for texture.\n");
		}
	}
	
	*id = texture_used;
}

void gfxActiveTexture(unsigned int texel)
{
	texel_active = texel;
	
	if(texel_active >= TEXTURE_UNITS)
	{
		texel_active = 0;
		gfxSetErrorMessage("Error: Accessing unavailable texture unit, only 8 (0 - 7) texture units available now.\n");
	}
}

void gfxBindTexture(unsigned int id)
{
	texture_active[texel_active] = id;
}

void gfxTexImage2D(enum texture_type type, unsigned int width, unsigned int height, void* data)
{
	if(texture_active[texel_active] == 0)
	{
		gfxSetErrorMessage("Error: Attemping to create texture on NULL texture.\n");
		return;
	}
	
	textures[texture_active[texel_active]].width = width;
	textures[texture_active[texel_active]].height = height;
	textures[texture_active[texel_active]].type = type;
	
	switch(type)
	{
		case GFX_RGBA8:
			textures[texture_active[texel_active]].pixel = sizeof(unsigned char) * 4;
			break;
			
		case GFX_RGBA32F:
			textures[texture_active[texel_active]].pixel = sizeof(float) * 4;
			break;
			
		case GFX_DEPTH32F:
			textures[texture_active[texel_active]].pixel = sizeof(float);
			break;
			
		default:
			gfxSetErrorMessage("Error: Invalid texture type.\n");
			return;
			break;
	}
	
	textures[texture_active[texel_active]].total = width * height * textures[texture_active[texel_active]].pixel;
	
	if(textures[texture_active[texel_active]].data != NULL)
	{
		gfx_free(textures[texture_active[texel_active]].data);
		textures[texture_active[texel_active]].data = NULL;
	}
	
	if(data == NULL)
	{
		textures[texture_active[texel_active]].data = gfx_alloc(textures[texture_active[texel_active]].total, 16);
		
		if(!textures[texture_active[texel_active]].data)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for texture data.\n");
		}
	}
	else
	{
		textures[texture_active[texel_active]].data = data;
	}
}

void gfxDeleteTexture(unsigned int id)
{
	gfx_free(textures[id].data);
	textures[id].data = NULL;
}
