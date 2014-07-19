#ifndef _TEXTURE_H
#define _TEXTURE_H

#include "gfx.h"

struct texture_t
{
	unsigned int width;
	unsigned int height;
	
	enum texture_type type;	
	
	size_t total;
	size_t pixel;
	
	void* data;
};

extern struct texture_t* textures;
extern unsigned int texture_active[];

#endif
