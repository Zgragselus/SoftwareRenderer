#ifndef _SHADERS_APP_H
#define _SHADERS_APP_H

#include "graphics/gfx.h"
#include "graphics/device.h"

/* Vertex shader */
void vsh_main(void* data)
{
	// Output position = projection matrix * model-view matrix * vertex position
	shd_varying0 = mul_mat_vec(uniform_matrix[0], mul_mat_vec(uniform_matrix[1], shd_input0));
	
	// Output texture coordinate
	shd_varying1 = shd_input1;
	
	// W division
	if(shd_varying0.fp.w != 0.0f)
	{
		shd_varying0.fp.x /= shd_varying0.fp.w;
		shd_varying0.fp.y /= shd_varying0.fp.w;
		shd_varying0.fp.z /= shd_varying0.fp.w;
	}
}

/* Pixel shader */
void fsh_main(void* data)
{
	// Output color - texture sample at coordinate
	shd_color = sampler2D(uniform_int[0], &shd_varying1);
	
	// Output depth - Z coordinate
	shd_depth = shd_varying0.fp.z;
}

#endif
