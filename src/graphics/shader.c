#include "gfx.h"
#include "device.h"
#include "shader.h"
#include "buffer.h"
#include "error.h"
#include "../memory/memory.h"

// Again this follows what is done in buffer
#define SHADER_BASE_COUNT 8

struct shader_t* shaders = NULL;
unsigned int shader_count = SHADER_BASE_COUNT + 1;
unsigned int shader_used = 0;
unsigned int shader_active = 0;

unsigned int gfxCreateShader()
{
	if(shaders == NULL)
	{
		shaders = malloc(sizeof(struct shader_t) * shader_count);
		
		if(!shaders)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for shader.\n");
		}
	}

	shader_used++;
	
	if(shader_used == shader_count)
	{
		shader_count *= 2;
		shaders = realloc(shaders, sizeof(struct shader_t) * shader_count);
		
		if(!shaders)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for shader.\n");
		}
	}
	
	return shader_used;
}

void gfxDeleteShader(unsigned int shader)
{
	shaders[shader].vertex_shader = NULL;
	shaders[shader].pixel_shader = NULL;
}

/* This procedure let us set shader entry points (procedures) */
void gfxShaderSource(unsigned int shader, void (*vertex_shader)(void* data), void (*pixel_shader)(void* data))
{
	if(shader_active == 0)
	{
		gfxSetErrorMessage("Error: Attemping to create shader on NULL shader.\n");
		return;
	}
	
	shaders[shader_active].vertex_shader = vertex_shader;
	shaders[shader_active].pixel_shader = pixel_shader;
	
	printf("%x %x\n", shaders[shader_active].vertex_shader, shaders[shader_active].pixel_shader);
}

void gfxUseShader(unsigned int program)
{
	shader_active = program;
}

/* Okay, we can have 1 buffer bind, so we need to somehow read packed data from it */
void gfxAttribPointer(unsigned int attrib_id, size_t item_size, size_t offset)
{
	_dev_shader_attribs.attribute_step[attrib_id] = item_size / sizeof(float4);
	_dev_shader_attribs.attribute[attrib_id] = ((float4*)buffers[buffer_active].data) + offset;
}

/* Just simple uniform setup for shaders */
void gfxUniform1i(unsigned int uniform_int_id, int value)
{
	uniform_int[uniform_int_id] = value;
}

void gfxUniform1f(unsigned int uniform_float_id, float value)
{
	uniform_float[uniform_float_id] = value;
}

void gfxUniform4f(unsigned int uniform_vector_id, float x, float y, float z, float w)
{
	uniform_vector[uniform_vector_id].fp.x = x;
	uniform_vector[uniform_vector_id].fp.y = y;
	uniform_vector[uniform_vector_id].fp.z = z;
	uniform_vector[uniform_vector_id].fp.w = w;
}

void gfxUniformMatrix4fv(unsigned int uniform_matrix_id, float* ptr)
{
	uniform_matrix[uniform_matrix_id].fp.m1 = _mm_setr_ps(ptr[0], ptr[1], ptr[2], ptr[3]);
	uniform_matrix[uniform_matrix_id].fp.m2 = _mm_setr_ps(ptr[4], ptr[5], ptr[6], ptr[7]);
	uniform_matrix[uniform_matrix_id].fp.m3 = _mm_setr_ps(ptr[8], ptr[9], ptr[10], ptr[11]);
	uniform_matrix[uniform_matrix_id].fp.m4 = _mm_setr_ps(ptr[12], ptr[13], ptr[14], ptr[15]);
}
