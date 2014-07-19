#ifndef _GFX_H
#define _GFX_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <xmmintrin.h>
#include <emmintrin.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GFX Library Objects																																																		   //

//////////////////////
/** TEXTURE OBJECT **/
enum texture_type
{
	GFX_RGBA8,
	GFX_RGBA32F,
	GFX_DEPTH32F
};

void gfxGenTexture(unsigned int* id);
void gfxActiveTexture(unsigned int texel);
void gfxBindTexture(unsigned int id);
void gfxTexImage2D(enum texture_type type, unsigned int width, unsigned int height, void* data);
void gfxDeleteTexture(unsigned int id);
//////////////////////

/////////////////////
/** BUFFER OBJECT **/
enum buffer_type
{
	GFX_ARRAY_BUFFER
};

void gfxGenBuffer(unsigned int* id);
void gfxBindBuffer(unsigned int id);
void gfxBufferData(enum buffer_type type, size_t items_size, void* data);
void gfxDeleteBuffer(unsigned int id);
void* gfxMapBuffer();
bool gfxUnmapBuffer();

enum draw_type
{
	GFX_POINTS,
	GFX_LINES,
	GFX_TRIANGLES
};

void gfxDrawArrays(enum draw_type draw, size_t offset, unsigned int count);
/////////////////////

/////////////////////
/** SHADER OBJECT **/
#define shd_input0 *(_dev_shader_attribs.attribute[0])
#define shd_input1 *(_dev_shader_attribs.attribute[1])
#define shd_input2 *(_dev_shader_attribs.attribute[2])
#define shd_input3 *(_dev_shader_attribs.attribute[3])

#define shd_varying0 _dev_shader_varyings.varyings[0]
#define shd_varying1 _dev_shader_varyings.varyings[1]
#define shd_varying2 _dev_shader_varyings.varyings[2]
#define shd_varying3 _dev_shader_varyings.varyings[3]

#define shd_color _dev_shader_output.color
#define shd_depth _dev_shader_output.depth

unsigned int gfxCreateShader();
void gfxDeleteShader(unsigned int shader);
void gfxShaderSource(unsigned int shader, void (*vertex_shader)(void* data), void (*pixel_shader)(void* data));
void gfxUseShader(unsigned int program);

void gfxUniform1i(unsigned int uniform_int_id, int value);
void gfxUniform1f(unsigned int uniform_float_id, float value);
void gfxUniform4f(unsigned int uniform_vector_id, float x, float y, float z, float w);
void gfxUniformMatrix4fv(unsigned int uniform_matrix_id, float* ptr);
void gfxUniformSampler(unsigned int uniform_sampler_id, unsigned int texture_unit);

void gfxAttribPointer(unsigned int attrib_id, size_t item_size, size_t offset);
/////////////////////

//////////////////////////
/** FRAMEBUFFER OBJECT **/
enum framebuffer_attachment
{
	GFX_COLOR_ATTACHMENT = 1,
	GFX_DEPTH_ATTACHMENT
};

void gfxGenFramebuffer(unsigned int* id);
void gfxBindFramebuffer(unsigned int id);
void gfxFramebufferAttachment(enum framebuffer_attachment attachment, unsigned int texture);
void gfxDeleteFramebuffer(unsigned int id);

void gfxSwapBuffers(unsigned int* width, unsigned int* height, void** data);
//////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GFX Library Common Procedures																																														 //

////////////////////////
/** BASIC OPERATIONS **/
void gfxClear();
void gfxClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void gfxClearColorf(float r, float g, float b, float a);
void gfxClearDepth(float depth);

char* gfxGetLastError();
////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
