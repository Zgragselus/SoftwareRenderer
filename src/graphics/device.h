#ifndef _DEVICE_H
#define _DEVICE_H

#include "../math/float4.h"
#include "../math/mat4.h"
#include "gfx.h"

// Shader attribute structure 
struct __attribute__((aligned(16))) shd_attribs
{
	float4 *attribute[4];
	size_t attribute_step[4];
};

extern struct shd_attribs _dev_shader_attribs;

// Shader varying structure
struct __attribute__((aligned(16))) shd_varying
{
	float4 varyings[4];
};

extern struct shd_varying _dev_shader_varyings;

// Shader output structure
struct __attribute__((aligned(16))) shd_output
{
	float4 color;
	float depth;
};

extern struct shd_output _dev_shader_output;

// Uniforms
extern int uniform_int[32];
extern float uniform_float[32];
extern float4 uniform_vector[32];
extern mat4 uniform_matrix[4];

// Procedures, for further description see device.c
void _dev_merge_output(int x, int y, float z, float4 color);
void _dev_rasterize_points(struct shd_varying* buffer, size_t offset, unsigned int count);
void _dev_rasterize_lines(struct shd_varying* buffer, size_t offset, unsigned int count);
void _dev_rasterize_triangles(struct shd_varying* buffer, size_t offset, unsigned int count);
void _dev_pipeline(enum draw_type draw, size_t offset, unsigned int count);

// Sampler procedure for shader
float4 sampler2D(unsigned int texture_unit, float4 *texcoords);

#endif
