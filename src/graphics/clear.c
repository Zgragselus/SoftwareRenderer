#include "gfx.h"
#include "clear.h"
#include "error.h"
#include "framebuffer.h"
#include "../memory/memory.h"

size_t color_attachment_size;
void* color_attachment;

size_t depth_attachment_size;
void* depth_attachment;

/* Clears the color & depth buffer, to colors set by procedures further */
void gfxClear()
{
	if(framebuffers[framebuffer_active].attachments & GFX_COLOR_ATTACHMENT)
	{
		size_t total = framebuffers[framebuffer_active].color_attachment->total;
		size_t done = 0;
		
		while(total > 0)
		{
			memcpy(framebuffers[framebuffer_active].color_attachment->data + done, color_attachment, color_attachment_size);
			done += color_attachment_size;
			total -= color_attachment_size;
		}
	}
	
	if(framebuffers[framebuffer_active].attachments & GFX_DEPTH_ATTACHMENT)
	{
		size_t total = framebuffers[framebuffer_active].depth_attachment->total;
		size_t done = 0;
		
		while(total > 0)
		{
			memcpy(framebuffers[framebuffer_active].depth_attachment->data + done, depth_attachment, depth_attachment_size);
			done += depth_attachment_size;
			total -= depth_attachment_size;
		}
	}
}

/* Set color on which we'll clear */
void gfxClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	if(color_attachment != NULL)
	{
		gfx_free(color_attachment);
		color_attachment = NULL;
	}
	
	color_attachment_size = sizeof(unsigned char) * 4;
	
	color_attachment = gfx_alloc(color_attachment_size, 16);
	
	if(!color_attachment)
	{
		gfxSetErrorMessage("Error: Out of memory. Unable to clear color.\n");
	}
	
	((unsigned char*)color_attachment)[0] = r;
	((unsigned char*)color_attachment)[1] = g;
	((unsigned char*)color_attachment)[2] = b;
	((unsigned char*)color_attachment)[3] = a;
}

/* Same as above, float version */
void gfxClearColorf(float r, float g, float b, float a)
{
	if(color_attachment != NULL)
	{
		gfx_free(color_attachment);
		color_attachment = NULL;
	}
	
	color_attachment_size = sizeof(float) * 4;
	
	color_attachment = gfx_alloc(color_attachment_size, 16);
	
	if(!color_attachment)
	{
		gfxSetErrorMessage("Error: Out of memory. Unable to clear color.\n");
	}
	
	((float*)color_attachment)[0] = r;
	((float*)color_attachment)[1] = g;
	((float*)color_attachment)[2] = b;
	((float*)color_attachment)[3] = a;
}

/* Clear depth, same as above - although it's about depth */
void gfxClearDepth(float depth)
{
	if(depth_attachment != NULL)
	{
		gfx_free(depth_attachment);
		depth_attachment = NULL;
	}
	
	depth_attachment_size = sizeof(float);
	
	depth_attachment = gfx_alloc(depth_attachment_size, 16);
	
	if(!depth_attachment)
	{
		gfxSetErrorMessage("Error: Out of memory. Unable to clear color.\n");
	}
	
	((float*)depth_attachment)[0] = depth;
}
