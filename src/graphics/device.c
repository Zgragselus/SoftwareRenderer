#include "device.h"
#include "buffer.h"
#include "framebuffer.h"
#include "texture.h"
#include "shader.h"
#include "gfx.h"
#include "../math/float4.h"
#include "../math/mat4.h"
#include "../memory/memory.h"

/* Pipeline goes like: (1) Input vertices -> (2) Vertex shader -> (3) Primitive assembly & Clipping & Rasterization -> (4) Pixel shader -> (5) Output merge */

struct shd_attribs _dev_shader_attribs;
struct shd_varying _dev_shader_varyings;
struct shd_output _dev_shader_output;

int uniform_int[32];
float uniform_float[32];
float4 uniform_vector[32];
mat4 uniform_matrix[4];

struct shd_varying *_dev_varyings;
unsigned int _dev_varyings_count;

void _dev_vertex_shader(unsigned int offset, unsigned int count)
{
	_dev_varyings_count = count;
	_dev_varyings = gfx_alloc(sizeof(struct shd_varying) * count, 16);
	
	for(unsigned int i = offset; i < count; i++)
	{
		shaders[shader_active].vertex_shader(NULL);
		
		_dev_varyings[i].varyings[0] = _dev_shader_varyings.varyings[0];
		_dev_varyings[i].varyings[1] = _dev_shader_varyings.varyings[1];
		_dev_varyings[i].varyings[2] = _dev_shader_varyings.varyings[2];
		_dev_varyings[i].varyings[3] = _dev_shader_varyings.varyings[3];
	
		_dev_shader_attribs.attribute[0] += _dev_shader_attribs.attribute_step[0];
		_dev_shader_attribs.attribute[1] += _dev_shader_attribs.attribute_step[1];
		_dev_shader_attribs.attribute[2] += _dev_shader_attribs.attribute_step[2];
		_dev_shader_attribs.attribute[3] += _dev_shader_attribs.attribute_step[3];
	}
}

void _dev_pixel_shader(int x, int y)
{
	shaders[shader_active].pixel_shader(NULL);
	
	_dev_merge_output(x, y, _dev_shader_output.depth, _dev_shader_output.color);
}

void _dev_merge_output(int x, int y, float z, float4 color)
{
	if(framebuffers[framebuffer_active].attachments & GFX_COLOR_ATTACHMENT)
	{
		switch(framebuffers[framebuffer_active].color_attachment->type)
		{
			case GFX_RGBA8:
				((unsigned char*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 0] = (unsigned char)(color.fp.x * 255.0f);
				((unsigned char*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 1] = (unsigned char)(color.fp.y * 255.0f);
				((unsigned char*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 2] = (unsigned char)(color.fp.z * 255.0f);
				((unsigned char*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 3] = (unsigned char)(color.fp.w * 255.0f);
				break;
		
			case GFX_RGBA32F:
				((float*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 0] = color.fp.x;
				((float*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 1] = color.fp.y;
				((float*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 2] = color.fp.z;
				((float*)framebuffers[framebuffer_active].color_attachment->data)[(y * framebuffers[framebuffer_active].color_attachment->width + x) * 4 + 3] = color.fp.w;
				break;
				
			default:
				break;
		}
	}
	
	if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
	{
		switch(framebuffers[framebuffer_active].depth_attachment->type)
		{
			case GFX_DEPTH32F:
				((float*)framebuffers[framebuffer_active].depth_attachment->data)[y * framebuffers[framebuffer_active].color_attachment->width + x] = z;
				break;
			
			default:
				break;
		}
	}
}

//////////////////////////
/* Points rasterization */

/* Rasterize set of points */
// @param1 - pointer to buffer containing varyings from vertex shader
// @param2 - offset of points into buffer
// @param3 - total count of points
void _dev_rasterize_points(struct shd_varying* buffer, size_t offset, unsigned int count)
{
	unsigned int steps = offset;																										// Index into points, where we start
	struct texture_t* result = framebuffers[framebuffer_active].color_attachment;		// Pointer to resulting color buffer
	struct texture_t* depth = framebuffers[framebuffer_active].depth_attachment;		// Pointer to resulting depth buffer
	
	// Loop through all points
	while(steps < count)
	{
		// Project points from post-projection space into viewport space
		int x = ((buffer[steps].varyings[0].fp.x * 0.5f + 0.5f) * (float)result->width);
		int y = ((buffer[steps].varyings[0].fp.y * 0.5f + 0.5f) * (float)result->height);
		
		// Clip points in viewport space
		if(x >= 0 && x < (int)result->width && y >= 0 && y < (int)result->height)
		{
			// Grab shader varyings for current point
			_dev_shader_varyings.varyings[0] = _dev_varyings[steps].varyings[0];
			_dev_shader_varyings.varyings[1] = _dev_varyings[steps].varyings[1];
			_dev_shader_varyings.varyings[2] = _dev_varyings[steps].varyings[2];
			_dev_shader_varyings.varyings[3] = _dev_varyings[steps].varyings[3];

			// If we have depth buffer attached, perform depth test
			if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
			{
				// If we pass the test, run pixel shader for this point
				float z = buffer[steps].varyings[0].fp.z;
				if(z >= 0.0f && z <= ((float*)depth->data)[y * depth->width + x])
				{
					_dev_pixel_shader(x, y);
				}
			}
			// If we don't have depth buffer attached, just run pixel shader for current point
			else
			{
				_dev_pixel_shader(x, y);
			}
			
		}
	
		// Loop through points
		steps++;
	}
}

/////////////////////////
/* Lines rasterization */

// First of all, when we rasterize lines, we can't be sure whether the line is whole in the viewport, or it goes over the 
// borders of the viewport. For this purpose we need to clip line to the viewport.

// Clipping the line is not as trivial as with points (where you just check whether point is in viewport. For lines, you have
// to test them against a viewport quad. For making this really simplier we'll work in so called clip-space, in this space the
// coordinates of screen goes from <-1.0, 1.0>.
// We don't have to invite some optimal algorithm for this (luckily for us), because there exists one - Cohen-Sutherland. It
// classifies both points of the line with bitmask (seen lower) and then tests line against only those edges that are needed.
// As soon as both points are in screen, we accept them - if they're not in the screen and won't be connected through it, we
// trivially reject the line.

/* Cohen sutherland clipping */
#define INSIDE 0 // 0000
#define LEFT 1   // 0001
#define RIGHT 2  // 0010
#define BOTTOM 4 // 0100
#define TOP 8    // 1000

/* Classify point */
int _dev_cs_code(float x, float y)
{
	int code = INSIDE;
	
	if(x < -1.0f)
	{
		code |= LEFT;
	}
	else if(x > 1.0f)
	{
		code |= RIGHT;
	}
	
	if(y < -1.0f)
	{
		code |= BOTTOM;
	}
	else if(y > 1.0f)
	{
		code |= TOP;
	}
	
	return code;
}

/* Clip the line */
// Note. we pass X and Y for both points and some L parameter - it's for having correct interpolation of colors, etc. over the 
// line (e.g. some interpolation value where 0.0 is base point1 and 1.0 is base point2).
bool _dev_cs_clip(float* x0, float* y0, float* l0, float* x1, float* y1, float* l1)
{
	// For start lets classify both points
	int code0 = _dev_cs_code(*x0, *y0);
	int code1 = _dev_cs_code(*x1, *y1);
	
	// And set that we reject the line (e.g. won't draw)
	bool accept = false;
	
	// Now let's enter infinite loop
	while(true)
	{
		// If (A OR B) == 0, both points are already in screen, so let's accept the line immediately
		if(!(code0 | code1))
		{
			accept = true;
			break;
		}
		// And if (A AND B) != 0, the line connecting both points won't be visible, so let's reject line immediately
		else if(code0 & code1)
		{
			break;
		}
		// Otherwise we have to clip
		else
		{
			float x, y, l;												// Temporary variables
			int outcode = code0 ? code0 : code1;	// Which of the points is away? (take 1st, first)
			
			// Now we clip the point against the edge against it needs to clip, this is quite simple mathematical way to 
			// solve this (2 line equations and finding their intersection)
			// Note. that we also store the interpolation value for later interpolating of varyings
			if(outcode & TOP)
			{
				x = *x0 + (*x1 - *x0) * (1.0f - *y0) / (*y1 - *y0);
				y = 1.0f;
				l = (1.0f - *y0) / (*y1 - *y0);
			}
			else if(outcode & BOTTOM)
			{
				x = *x0 + (*x1 - *x0) * (-1.0f - *y0) / (*y1 - *y0);
				y = -1.0f;
				l = (-1.0f - *y0) / (*y1 - *y0);
			}
			else if(outcode & RIGHT)
			{
				x = 1.0f;
				y = *y0 + (*y1 - *y0) * (1.0f - *x0) / (*x1 - *x0);
				l = (1.0f - *x0) / (*x1 - *x0);
			}
			else
			{
				x = -1.0f;
				y = *y0 + (*y1 - *y0) * (-1.0f - *x0) / (*x1 - *x0);
				l = (-1.0f - *x0) / (*x1 - *x0);
			}
			
			// Now store the clipped point and classify it again
			if(outcode == code0)
			{
				*x0 = x;
				*y0 = y;
				*l0 = l;
				code0 = _dev_cs_code(*x0, *y0);
			}
			else
			{
				*x1 = x;
				*y1 = y;
				*l1 = l;
				code1 = _dev_cs_code(*x1, *y1);
			}
		}
	}
	
	// Return whether we accepted or rejected line
	return accept;
}

/* Rasterize set of points as lines */
// @param1 - pointer to buffer containing varyings from vertex shader
// @param2 - offset of points into buffer
// @param3 - total count of points
void _dev_rasterize_lines(struct shd_varying* buffer, size_t offset, unsigned int count)
{
	unsigned int steps = 2 * offset;																								// How many points have we already done
	struct texture_t* result = framebuffers[framebuffer_active].color_attachment;		// Store pointer to color buffer
	struct texture_t* depth = framebuffers[framebuffer_active].depth_attachment;		// Store pointer to depth buffer
	
	// Loop through points (take 2 at once, because 2 points form a line)
	while(steps < count)
	{
		// Get first point, set it's interpolation value to 0.0
		float x0 = buffer[steps].varyings[0].fp.x;
		float y0 = buffer[steps].varyings[0].fp.y;
		float l0 = 0.0f;
		
		// Get second point, set it's interpolation value to 1.0
		float x1 = buffer[steps + 1].varyings[0].fp.x;
		float y1 = buffer[steps + 1].varyings[0].fp.y;
		float l1 = 1.0f;
	
		// Clip the line to screen
		bool accept = _dev_cs_clip(&x0, &y0, &l0, &x1, &y1, &l1);
		
		// If the line is accepted
		if(accept)
		{
			// Now let's rasterize the line with something called Bresenham's line rasterization algorithm
			// Calculate point coordinates in screen-space
			int _x0 = (int)((x0 * 0.5f + 0.5f) * (float)result->width);
			int _x1 = (int)((x1 * 0.5f + 0.5f) * (float)result->width);
			int _y0 = (int)((y0 * 0.5f + 0.5f) * (float)result->height);
			int _y1 = (int)((y1 * 0.5f + 0.5f) * (float)result->height);
			
			// Where are we currently plotting (temp variables)
			int x = _x0;
			int y = _y0;
			
			// Delta X and Y for line
			int dx = _x1 - _x0;
			if(dx < 0)
				dx = -dx;
			int dy = _y1 - _y0;
			if(dy < 0)
				dy = -dy;

			// Sign - e.g. directions in which we'll move
			int sx = -1;
			if(_x0 < _x1)
				sx = 1;
			int sy = -1;
			if(_y0 < _y1)
				sy = 1;
			
			// Our temporary value for interpolation
			float l = l0;
			
			// Now we'll always step in one direction and sometimes in another one. Let's separate both cases (X and Y).
			// If we always step in X
			if(dx > dy)
			{
				// These values are so called error holders, if error (p) grows over 0, we step 1 pixel in Y direction, otherwise we go
				// just in X direction
				int dpr = dy << 1;
				int dpru = dpr - (dx << 1);
				int p = dpr - dx;
				
				// And our interpolator step
				float lstep = (l1 - l0) / (float)(abs(_x1 - _x0));
				
				for(; dx >= 0; dx--)
				{
					_dev_shader_varyings.varyings[0].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[0].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[0].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[1].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[1].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[1].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[2].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[2].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[2].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[3].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[3].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[3].xmm, _mm_set1_ps(l)));
				
					if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
					{
						float z = (1.0f - l) * buffer[steps + 0].varyings[0].fp.z + l * buffer[steps + 1].varyings[0].fp.z;
						if(z >= 0.0f && z <= ((float*)depth->data)[y * depth->width + x])
						{
							_dev_pixel_shader(x, y);
						}
					}
					else
					{
						_dev_pixel_shader(x, y);
					}
					
					l += lstep;
					
					// If error is bigger than 0, step also in Y direction, otherwise just X
					if(p > 0)
					{
						x += sx;
						y += sy;
						p += dpru;
					}
					else
					{
						x += sx;
						p += dpr;
					}
				}
			}
			// Otherwise we actually do *the same* but stepping always in Y direction
			else
			{
				int dpr = dx << 1;
				int dpru = dpr - (dy << 1);
				int p = dpr - dy;
				float lstep = (l1 - l0) / (float)(abs(_y1 - _y0));
				
				for(; dy >= 0; dy--)
				{
					_dev_shader_varyings.varyings[0].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[0].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[0].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[1].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[1].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[1].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[2].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[2].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[2].xmm, _mm_set1_ps(l)));
					_dev_shader_varyings.varyings[3].xmm = _mm_add_ps(_mm_mul_ps(_dev_varyings[steps].varyings[3].xmm, _mm_set1_ps(1.0f - l)), _mm_mul_ps(_dev_varyings[steps + 1].varyings[3].xmm, _mm_set1_ps(l)));
				
					if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
					{
						float z = (1.0f - l) * buffer[steps + 0].varyings[0].fp.z + l * buffer[steps + 1].varyings[0].fp.z;
						if(z >= 0.0f && z <= ((float*)depth->data)[y * depth->width + x])
						{
							_dev_pixel_shader(x, y);
						}
					}
					else
					{
						_dev_pixel_shader(x, y);
					}
					
					l += lstep;
					
					if(p > 0)
					{
						x += sx;
						y += sy;
						p += dpru;
					}
					else
					{
						y += sy;
						p += dpr;
					}
				}
			}
		}
		
		// Step 2 points in buffer
		steps += 2;
	}
}

////////////////////////////
/* Triangle rasterization */

// For triangle rasterization we use something called halfspace triangle raterization. We determine whether the point is inside
// of triangle by getting whether it's on positive side of all half spaces generated by triangle's edges. Also we compute
// barycentric coordinate for triangle to correctly interpolate values over triangle.

// As for triangle clipping, it was quite complicated for lines, and it's suprisingly even more complicated for triangles.
// There is so called Sutherland-Hodgman algorithm to clip polygon against viewport, and this is where it gets complicated and
// a lot. When we clipped line against viewport, we got either a line or nothing to draw, with triangles it's quite unfortunate
// that we can get either a triangle, or N-gon (maximally seven-gon), and rendering polygons is suprisingly even more complex
// than rendering triangles.
// While it seems that now we're screwed, it's not true - because we can be assured that resulting polygon is convex (and thats
// really important). Each 3 neighbouring points in polygon are forming a triangle! And thats it - we just take the input point
// array, take always triangle (3 verts from it), clip them against viewport and then put all the generated triangles into 
// second array, that will we rasterize.

// So how to clip triangle against viewport - we can re-use the Cohen-Sutherland codes to classify trivial accept/rejection of
// triangle (this give us quite huge improvement of performance). And we can use Cohen-Sutherland clipping to clip all the 
// edges of the triangle against viewport - then it's quite straight forward to generate triangles out of these.

/* Helper procedure for resizing dynamic buffer */
void _dev_tri_clip_check_resize(struct shd_varying** buffer, int* buffer_count, int* buffer_used)
{
	if(*buffer_used + 1 > *buffer_count)
	{
		unsigned int temp = *buffer_count * 2;
		struct shd_varying* temp_buffer = gfx_alloc(sizeof(struct shd_varying) * temp, 16);
		memcpy(temp_buffer, *buffer, *buffer_count * sizeof(struct shd_varying));
		gfx_free(*buffer);
		*buffer = temp_buffer;
		*buffer_count = temp;
	}
}

/* Helper procedure for appending value onto dynamic buffer */
void _dev_tri_clip_append(struct shd_varying** buffer, int* buffer_count, int* buffer_used, struct shd_varying* value)
{
	_dev_tri_clip_check_resize(buffer, buffer_count, buffer_used);
	
	for(int i = 0; i < 4; i++)
	{
		(*buffer)[*buffer_used].varyings[i].xmm = value->varyings[i].xmm;
	}
	
	(*buffer_used)++;
}

/* Helper procedure for clipping state of point */
bool _dev_tri_clip_state(struct shd_varying* point, int edge)
{
	switch(edge)
	{
		case 0:
			if(point->varyings[0].fp.x < -1.0f)
			{
				return false;
			}
			break;
			
		case 1:
			if(point->varyings[0].fp.x > 1.0f)
			{
				return false;
			}
			break;
			
		case 2:
			if(point->varyings[0].fp.y < -1.0f)
			{
				return false;
			}
			break;
			
		case 3:
			if(point->varyings[0].fp.y > 1.0f)
			{
				return false;
			}
			break;
			
		default:
			break;
	}
	
	return true;
}

/* Triangle clipping algorithm */
void _dev_cs_tri_clip(struct shd_varying* buffer, struct shd_varying** output, int* output_count)
{
	// As described upper - this is sutherland hodgman algorithm
	*output_count = 0;
	*output = NULL;

	unsigned int steps = 0;
	while(steps < _dev_varyings_count)
	{
		int out_count = 3;
		int out_used = 0;
		struct shd_varying* out_buffer = gfx_alloc(sizeof(struct shd_varying) * out_count, 16);
		_dev_tri_clip_append(&out_buffer, &out_count, &out_used, &(buffer[steps]));
		_dev_tri_clip_append(&out_buffer, &out_count, &out_used, &(buffer[steps + 1]));
		_dev_tri_clip_append(&out_buffer, &out_count, &out_used, &(buffer[steps + 2]));
		
		for(int edge = 0; edge < 4; edge++)
		{
			int in_count = out_used;
			int in_used = 0;
			struct shd_varying* in_buffer = gfx_alloc(sizeof(struct shd_varying) * in_count, 16);
			for(int i = 0; i < out_used; i++)
			{
				_dev_tri_clip_append(&in_buffer, &in_count, &in_used, &(out_buffer[i]));
			}
			
			out_used = 0;
			
			struct shd_varying* pt_start = &(in_buffer[in_used - 1]);
			for(int i = 0; i < in_count; i++)
			{
				struct shd_varying* pt_current = &(in_buffer[i]);
				
				bool pt_current_inside = _dev_tri_clip_state(pt_current, edge);
				bool pt_start_inside = _dev_tri_clip_state(pt_start, edge);
				
				if(pt_current_inside)
				{
					if(!pt_start_inside)
					{
						float x, y, l;
						switch(edge)
						{
							case 0:
								x = -1.0f;
								y = pt_current->varyings[0].fp.y + (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y) * (-1.0f - pt_current->varyings[0].fp.x) / (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.y);
								l = (-1.0f - pt_current->varyings[0].fp.x) / (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.x);
								break;
								
							case 1:
								x = 1.0f;
								y = pt_current->varyings[0].fp.y + (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y) * (1.0f - pt_current->varyings[0].fp.x) / (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.x);
								l = (1.0f - pt_current->varyings[0].fp.x) / (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.x);
								break;
								
							case 2:
								x = pt_current->varyings[0].fp.x + (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.x) * (-1.0f - pt_current->varyings[0].fp.y) / (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y);
								y = -1.0f;
								l = (-1.0f - pt_current->varyings[0].fp.y) / (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y);
								break;
								
							case 3:
								x = pt_current->varyings[0].fp.x + (pt_start->varyings[0].fp.x - pt_current->varyings[0].fp.x) * (1.0f - pt_current->varyings[0].fp.y) / (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y);
								y = 1.0f;
								l = (1.0f - pt_current->varyings[0].fp.y) / (pt_start->varyings[0].fp.y - pt_current->varyings[0].fp.y);
								break;
						}
						
						struct shd_varying append;
						for(int v = 0; v < 4; v++)
						{
							if(v == 0)
							{
								append.varyings[v].fp.x = (1.0f - l) * pt_current->varyings[v].fp.x + l * pt_start->varyings[v].fp.x;
								append.varyings[v].fp.y = (1.0f - l) * pt_current->varyings[v].fp.y + l * pt_start->varyings[v].fp.y;
								append.varyings[v].fp.z = (1.0f - l) * pt_current->varyings[v].fp.z + l * pt_start->varyings[v].fp.z;
								append.varyings[v].fp.w = (1.0f - l) * pt_current->varyings[v].fp.w + l * pt_start->varyings[v].fp.w;
							}
							else
							{
								// Note. everything except position has to be perspective-corrected
								float w = 1.0f / ((1.0f - l) * 1.0f / pt_current->varyings[0].fp.w + l * 1.0f / pt_start->varyings[0].fp.w);
								append.varyings[v].fp.x = ((1.0f - l) * pt_current->varyings[v].fp.x / pt_current->varyings[0].fp.w + l * pt_start->varyings[v].fp.x / pt_start->varyings[0].fp.w) * w;
								append.varyings[v].fp.y = ((1.0f - l) * pt_current->varyings[v].fp.y / pt_current->varyings[0].fp.w + l * pt_start->varyings[v].fp.y / pt_start->varyings[0].fp.w) * w;
								append.varyings[v].fp.z = ((1.0f - l) * pt_current->varyings[v].fp.z / pt_current->varyings[0].fp.w + l * pt_start->varyings[v].fp.z / pt_start->varyings[0].fp.w) * w;
								append.varyings[v].fp.w = (1.0f - l) * pt_current->varyings[v].fp.w + l * pt_start->varyings[v].fp.w;
							}
						}
						
						_dev_tri_clip_append(&out_buffer, &out_count, &out_used, &append);
					}
					
					_dev_tri_clip_append(&out_buffer, &out_count, &out_used, pt_current);
				}
				else if(pt_start_inside)
				{
					float x, y, l;
					switch(edge)
					{
						case 0:
							x = -1.0f;
							y = pt_start->varyings[0].fp.y + (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y) * (-1.0f - pt_start->varyings[0].fp.x) / (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.y);
							l = (-1.0f - pt_start->varyings[0].fp.x) / (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.x);
							break;
							
						case 1:
							x = 1.0f;
							y = pt_start->varyings[0].fp.y + (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y) * (1.0f - pt_start->varyings[0].fp.x) / (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.x);
							l = (1.0f - pt_start->varyings[0].fp.x) / (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.x);
							break;
							
						case 2:
							x = pt_start->varyings[0].fp.x + (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.x) * (-1.0f - pt_start->varyings[0].fp.y) / (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y);
							y = -1.0f;
							l = (-1.0f - pt_start->varyings[0].fp.y) / (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y);
							break;
							
						case 3:
							x = pt_start->varyings[0].fp.x + (pt_current->varyings[0].fp.x - pt_start->varyings[0].fp.x) * (1.0f - pt_start->varyings[0].fp.y) / (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y);
							y = 1.0f;
							l = (1.0f - pt_start->varyings[0].fp.y) / (pt_current->varyings[0].fp.y - pt_start->varyings[0].fp.y);
							break;
					}
					
					struct shd_varying append;
					for(int v = 0; v < 4; v++)
					{
						if(v == 0)
						{
							append.varyings[v].fp.x = (1.0f - l) * pt_start->varyings[v].fp.x + l * pt_current->varyings[v].fp.x;
							append.varyings[v].fp.y = (1.0f - l) * pt_start->varyings[v].fp.y + l * pt_current->varyings[v].fp.y;
							append.varyings[v].fp.z = (1.0f - l) * pt_start->varyings[v].fp.z + l * pt_current->varyings[v].fp.z;
							append.varyings[v].fp.w = (1.0f - l) * pt_start->varyings[v].fp.w + l * pt_current->varyings[v].fp.w;
						}
						else
						{
							// Note. everything except position has to be perspective-corrected
							float w = 1.0f / ((1.0f - l) * 1.0f / pt_start->varyings[0].fp.w + l * 1.0f / pt_current->varyings[0].fp.w);
							append.varyings[v].fp.x = ((1.0f - l) * pt_start->varyings[v].fp.x / pt_start->varyings[0].fp.w + l * pt_current->varyings[v].fp.x / pt_current->varyings[0].fp.w) * w;
							append.varyings[v].fp.y = ((1.0f - l) * pt_start->varyings[v].fp.y / pt_start->varyings[0].fp.w + l * pt_current->varyings[v].fp.y / pt_current->varyings[0].fp.w) * w;
							append.varyings[v].fp.z = ((1.0f - l) * pt_start->varyings[v].fp.z / pt_start->varyings[0].fp.w + l * pt_current->varyings[v].fp.z / pt_current->varyings[0].fp.w) * w;
							append.varyings[v].fp.w = (1.0f - l) * pt_start->varyings[v].fp.w + l * pt_current->varyings[v].fp.w;
						}
					}
					
					_dev_tri_clip_append(&out_buffer, &out_count, &out_used, &append);
				}
				
				pt_start = pt_current;
			}
		
			gfx_free(in_buffer);
		}
		
		unsigned int base_count = *output_count;
		
		/* Triangulize polygon */
		if((out_used - 2) * 3 > 0)
		{
			if((*output) == NULL)
			{
				*output = gfx_alloc(sizeof(struct shd_varying) * (*output_count + (out_used - 2) * 3), 16);
				*output_count += (out_used - 2) * 3;
			}
			else
			{
				struct shd_varying* temp = gfx_alloc(sizeof(struct shd_varying) * (*output_count + (out_used - 2) * 3), 16);
				memcpy(temp, *output, sizeof(struct shd_varying) * (*output_count));
				gfx_free(*output);
				*output = temp;
				
				*output_count += (out_used - 2) * 3;
			}
			
			unsigned int base = 0;
			unsigned int tmp = 1;
			
			for(int i = 0; i < (out_used - 2) * 3; i+= 3)
			{
				(*output)[base_count + i].varyings[0].xmm = out_buffer[base].varyings[0].xmm;
				(*output)[base_count + i].varyings[1].xmm = out_buffer[base].varyings[1].xmm;
				(*output)[base_count + i].varyings[2].xmm = out_buffer[base].varyings[2].xmm;
				(*output)[base_count + i].varyings[3].xmm = out_buffer[base].varyings[3].xmm;
				
				(*output)[base_count + i + 1].varyings[0].xmm = out_buffer[tmp].varyings[0].xmm;
				(*output)[base_count + i + 1].varyings[1].xmm = out_buffer[tmp].varyings[1].xmm;
				(*output)[base_count + i + 1].varyings[2].xmm = out_buffer[tmp].varyings[2].xmm;
				(*output)[base_count + i + 1].varyings[3].xmm = out_buffer[tmp].varyings[3].xmm;
				
				(*output)[base_count + i + 2].varyings[0].xmm = out_buffer[tmp + 1].varyings[0].xmm;
				(*output)[base_count + i + 2].varyings[1].xmm = out_buffer[tmp + 1].varyings[1].xmm;
				(*output)[base_count + i + 2].varyings[2].xmm = out_buffer[tmp + 1].varyings[2].xmm;
				(*output)[base_count + i + 2].varyings[3].xmm = out_buffer[tmp + 1].varyings[3].xmm;
				
				tmp++;
			}
		}
		
		gfx_free(out_buffer);
		
		steps += 3;
	}
}

/* Halfspace triangle rasterization */
void _dev_rasterize_triangles(struct shd_varying* buffer, size_t offset, unsigned int count)
{
	unsigned int steps = 3 * offset;																								// How many points have we already done
	struct texture_t* result = framebuffers[framebuffer_active].color_attachment;		// Store pointer to color buffer
	struct texture_t* depth = framebuffers[framebuffer_active].depth_attachment;		// Store pointer to depth buffer
	
	// Clipping
	struct shd_varying* clipped = NULL;
	int clipped_count = 0;
	_dev_cs_tri_clip(buffer, &clipped, &clipped_count);
	
	while(steps < clipped_count)
	{
		// Halfspace triangle rasterization
		int x1 = (int)((clipped[steps].varyings[0].fp.x * 0.5f + 0.5f) * (float)result->width);
		int x2 = (int)((clipped[steps + 1].varyings[0].fp.x * 0.5f + 0.5f) * (float)result->width);
		int x3 = (int)((clipped[steps + 2].varyings[0].fp.x * 0.5f + 0.5f) * (float)result->width);
		int y1 = (int)((clipped[steps].varyings[0].fp.y * 0.5f + 0.5f) * (float)result->height);
		int y2 = (int)((clipped[steps + 1].varyings[0].fp.y * 0.5f + 0.5f) * (float)result->height);
		int y3 = (int)((clipped[steps + 2].varyings[0].fp.y * 0.5f + 0.5f) * (float)result->height);
		
		int dx12 = x1 - x2;
		int dx23 = x2 - x3;
		int dx31 = x3 - x1;
		
		int dy12 = y1 - y2;
		int dy23 = y2 - y3;
		int dy31 = y3 - y1;
		
    int minx = x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3);
    int maxx = x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3);
    int miny = y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);
    int maxy = y1 > y2 ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3);
    
    int c1 = dy12 * x1 - dx12 * y1;
    int c2 = dy23 * x2 - dx23 * y2;
    int c3 = dy31 * x3 - dx31 * y3;
		
    int cy1 = c1 + dx12 * miny - dy12 * minx;
    int cy2 = c2 + dx23 * miny - dy23 * minx;
    int cy3 = c3 + dx31 * miny - dy31 * minx;

    for(int y = miny; y < maxy; y++)
    {
      int cx1 = cy1;
      int cx2 = cy2;
			int cx3 = cy3;
			
      for(int x = minx; x < maxx; x++)
      {
      	if(cx1 <= 0 && cx2 <= 0 && cx3 <= 0)
  		  {
  		  	float v0x = x3 - x1; float v0y = y3 - y1;
  		  	float v1x = x2 - x1; float v1y = y2 - y1;
  		  	float v2x = x - x1; float v2y = y - y1;
  		  	
  		  	float d00 = v0x * v0x + v0y * v0y;
  		  	float d01 = v0x * v1x + v0y * v1y;
  		  	float d02 = v0x * v2x + v0y * v2y;
  		  	float d11 = v1x * v1x + v1y * v1y;
  		  	float d12 = v1x * v2x + v1y * v2y;
  		  	
  		  	float invd = 1.0f / (d00 * d11 - d01 * d01);
  		  	float u = (d11 * d02 - d01 * d12) * invd;
  		  	float v = (d00 * d12 - d01 * d02) * invd;
		
			  	float b1 = u;
  		  	float b2 = v;
  		  	float b3 = 1.0f - u - v;
  		  	
  		  	/* Now comes per-pixel perspective correct coordinate computation. Let us have 3 vertices v1, v2 and v3.
  		  	   Barycentric coordinates b1, b2 and b3. Now the coordinates are:
  		  	   
						 w' = ( 1 / v1.w ) * b1 + ( 1 / v2.w ) * b2 + ( 1 / v3.w ) * b3
						 
						 u' = ( v1.u / v1.w ) * b1 + ( v2.u / v2.w ) * b2 + ( v3.u / v3.w ) * b3
						 v' = ( v1.v / v1.w ) * b1 + ( v2.v / v2.w ) * b2 + ( v3.v / v3.w ) * b3
						 
						 perspCorrU = u' / w'
						 perspCorrV = v' / w' */

					float w = (1.0f / clipped[steps].varyings[0].fp.w) * b3 + (1.0f / clipped[steps + 1].varyings[0].fp.w) * b2 + (1.0f / clipped[steps + 2].varyings[0].fp.w) * b1;
					
					for(int i = 0; i < 4; i++)
					{
				  	_dev_shader_varyings.varyings[i].fp.x = clipped[steps].varyings[i].fp.x / clipped[steps].varyings[0].fp.w * b3 + 
				  																					clipped[steps + 1].varyings[i].fp.x / clipped[steps + 1].varyings[0].fp.w * b2 + 
				  																					clipped[steps + 2].varyings[i].fp.x / clipped[steps + 2].varyings[0].fp.w * b1;
						_dev_shader_varyings.varyings[i].fp.x /= w;
				  																					
				  	_dev_shader_varyings.varyings[i].fp.y = clipped[steps].varyings[i].fp.y / clipped[steps].varyings[0].fp.w * b3 + 
				  																					clipped[steps + 1].varyings[i].fp.y / clipped[steps + 1].varyings[0].fp.w * b2 + 
				  																					clipped[steps + 2].varyings[i].fp.y / clipped[steps + 2].varyings[0].fp.w * b1;
						_dev_shader_varyings.varyings[i].fp.y /= w;
				  																					
				  	_dev_shader_varyings.varyings[i].fp.z = clipped[steps].varyings[i].fp.z / clipped[steps].varyings[0].fp.w * b3 + 
				  																					clipped[steps + 1].varyings[i].fp.z / clipped[steps + 1].varyings[0].fp.w * b2 + 
				  																					clipped[steps + 2].varyings[i].fp.z / clipped[steps + 2].varyings[0].fp.w * b1;
						_dev_shader_varyings.varyings[i].fp.z /= w;
				  																					
				  	_dev_shader_varyings.varyings[i].fp.w = clipped[steps].varyings[i].fp.w / clipped[steps].varyings[0].fp.w * b3 + 
				  																					clipped[steps + 1].varyings[i].fp.w / clipped[steps + 1].varyings[0].fp.w * b2 + 
				  																					clipped[steps + 2].varyings[i].fp.w / clipped[steps + 2].varyings[0].fp.w * b1;
						_dev_shader_varyings.varyings[i].fp.w /= w;
					}
  		  	
  		  	// Now comes output
  		  	
					if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
					{
						// If we have depth buffer, do early Z - and don't run pixel shaders if not necessary
						float z = clipped[steps].varyings[0].fp.z * b3 + clipped[steps + 1].varyings[0].fp.z * b2 + clipped[steps + 2].varyings[0].fp.z * b1;
						if(z >= 0.0f && z <= ((float*)depth->data)[y * depth->width + x])
						{
							_dev_pixel_shader(x, y);
						}
					}
					else
					{
						_dev_pixel_shader(x, y);
					}
  		  }
  		  
  		  cx1 -= dy12;
        cx2 -= dy23;
				cx3 -= dy31;
      }
      
      cy1 += dx12;
      cy2 += dx23;
      cy3 += dx31;
    }
    
		steps += 3;
	}
	
	if(clipped)
	{
		gfx_free(clipped);
		clipped = NULL;
	}
}

/* Pipe-line */
void _dev_pipeline(enum draw_type draw, size_t offset, unsigned int count)
{
	// Vertex shader first
	_dev_vertex_shader(offset, count);
	
	// Rasterization & clipping next, based upon primitive (this is primitive assembly!)
	switch(draw)
	{
		case GFX_POINTS:
			_dev_rasterize_points(_dev_varyings, offset, count);
			break;
			
		case GFX_LINES:
			_dev_rasterize_lines(_dev_varyings, offset, count);
			break;
			
		case GFX_TRIANGLES:
			_dev_rasterize_triangles(_dev_varyings, offset, count);
			break;
	}
	
	gfx_free(_dev_varyings);
	_dev_varyings = NULL;
	
	_dev_varyings_count = 0;
}

/* Texture sampler */
float4 sampler2D(unsigned int texture_unit, float4 *texcoords)
{
	// Just simple grab texture pointer, compute coordinates and output pixel color
	struct texture_t* tex = &(textures[texture_active[texture_unit]]);

	float u_fp = texcoords->fp.x - floorf(texcoords->fp.x);
	float v_fp = texcoords->fp.y - floorf(texcoords->fp.y);
	
	int u = (int)(u_fp * (float)tex->width);
	int v = (int)(v_fp * (float)tex->height);
	int coord = v * tex->width + u;
	
	float4 output;
	
	switch(tex->type)
	{
		case GFX_RGBA8:
			coord *= 4;
			output.fp.x = (float)(((unsigned char*)tex->data)[coord + 0]) / 255.0f;
			output.fp.y = (float)(((unsigned char*)tex->data)[coord + 1]) / 255.0f;
			output.fp.z = (float)(((unsigned char*)tex->data)[coord + 2]) / 255.0f;
			output.fp.w = (float)(((unsigned char*)tex->data)[coord + 3]) / 255.0f;
			break;
			
		case GFX_RGBA32F:
			coord *= 4;
			output.fp.x = ((float*)tex->data)[coord + 0];
			output.fp.y = ((float*)tex->data)[coord + 1];
			output.fp.z = ((float*)tex->data)[coord + 2];
			output.fp.w = ((float*)tex->data)[coord + 3];
			break;
	}
	
	return output;
}
