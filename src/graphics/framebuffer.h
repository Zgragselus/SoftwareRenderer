#ifndef _FRAMEBUFFER_H
#define _FRAMEBUFFER_H

#include "texture.h"

/* Framebuffer structure */
struct framebuffer_t
{
	enum framebuffer_attachment attachments;
	
	struct texture_t* color_attachment;
	struct texture_t* depth_attachment;
};

// List of framebuffers, and active framebuffer id
extern struct framebuffer_t* framebuffers;
extern unsigned int framebuffer_active;

#endif
