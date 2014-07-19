#include "gfx.h"
#include "error.h"
#include "framebuffer.h"

// Basically this follows what is done in buffer
#define FRAMEBUFFER_BASE_COUNT 8
struct framebuffer_t* framebuffers = NULL;
unsigned int framebuffer_count = FRAMEBUFFER_BASE_COUNT + 1;
unsigned int framebuffer_used = 0;
unsigned int framebuffer_active = 0;

void gfxGenFramebuffer(unsigned int* id)
{
	if(framebuffers == NULL)
	{
		framebuffers = malloc(sizeof(struct framebuffer_t) * framebuffer_count);
		
		if(!framebuffers)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for framebuffer.\n");
		}
	}

	framebuffer_used++;
	
	if(framebuffer_used == framebuffer_count)
	{
		framebuffer_count *= 2;
		framebuffer_count += 1;
		framebuffers = realloc(framebuffers, sizeof(struct framebuffer_t) * framebuffer_count);
		
		if(!framebuffers)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for framebuffer.\n");
		}
	}
	
	*id = framebuffer_used;
	framebuffers[framebuffer_used].attachments = 0;
}

void gfxBindFramebuffer(unsigned int id)
{
	framebuffer_active = id;
}

/* Except this procedure - it attaches textures to framebuffer */
void gfxFramebufferAttachment(enum framebuffer_attachment attachment, unsigned int texture)
{
	if(framebuffer_active == 0)
	{
		gfxSetErrorMessage("Error: Attemping to attach texture to NULL framebuffer.\n");
		return;
	}
	
	framebuffers[framebuffer_active].attachments |= attachment;
	
	switch(attachment)
	{
		case GFX_COLOR_ATTACHMENT:
			framebuffers[framebuffer_active].color_attachment = &(textures[texture]);
			break;
			
		case GFX_DEPTH_ATTACHMENT:
			framebuffers[framebuffer_active].depth_attachment = &(textures[texture]);
			break;
	}
}

void gfxDeleteFramebuffer(unsigned int id)
{
	framebuffers[id].color_attachment = NULL;
	framebuffers[id].depth_attachment = NULL;
}

/* We need somehow to read back-buffer this procedure name is although probably wrong, it should be rather gfxGetBuffersPointer */
void gfxSwapBuffers(unsigned int* width, unsigned int* height, void** data)
{
	if(framebuffer_active == 0)
	{
		gfxSetErrorMessage("Error: Can't swap buffers from NULL framebuffer.\n");
		return;
	}
	
	if(framebuffers[framebuffer_active].attachments & GFX_COLOR_ATTACHMENT)
	{
		*width = framebuffers[framebuffer_active].color_attachment->width;
		*height = framebuffers[framebuffer_active].color_attachment->height;
		*data = framebuffers[framebuffer_active].color_attachment->data;
	}
	else
	{
		gfxSetErrorMessage("Error: Can't swap buffers with framebuffer without color attachment.\n");
		return;
	}
}
