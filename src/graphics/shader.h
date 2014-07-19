#ifndef _SHADER_H
#define _SHADER_H

#include "../math/float4.h"

/* Shader structure & externs */
struct shader_t
{
	void (*vertex_shader)(void*);
	void (*pixel_shader)(void*);
};

extern struct shader_t* shaders;
extern unsigned int shader_count;
extern unsigned int shader_used;
extern unsigned int shader_active;

#endif
