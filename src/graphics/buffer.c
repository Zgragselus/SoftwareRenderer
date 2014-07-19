#include "gfx.h"
#include "buffer.h"
#include "error.h"
#include "device.h"
#include "../memory/memory.h"

#define BUFFER_BASE_COUNT 64

// Basically it's pointer to buffers array and variables used for rellocating + for active buffer (state variable)
struct buffer_t* buffers = NULL;
unsigned int buffer_count = BUFFER_BASE_COUNT + 1;
unsigned int buffer_used = 0;
unsigned int buffer_active = 0;

/* Gen buffer procedure */
void gfxGenBuffer(unsigned int* id)
{
	// If no buffers allocated yet, allocate
	if(buffers == NULL)
	{
		buffers = malloc(sizeof(struct buffer_t) * buffer_count);
		
		for(int i = 0; i < buffer_count; i++)
		{
			buffers[i].data = NULL;
		}
		
		if(!buffers)
		{	
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for buffer.\n");
		}
	}

	// Increment buffers used variable and realloc if needed
	buffer_used++;
	
	if(buffer_used == buffer_count)
	{
		buffer_count *= 2;
		buffer_count += 1;
		
		buffers = realloc(buffers, sizeof(struct buffer_t) * buffer_count);
		
		for(int i = 0; i < buffer_count; i++)
		{
			buffers[i].data = NULL;
		}
		
		if(!buffers)
		{
			gfxSetErrorMessage("Error: Out of memory. Unable to allocate data for texture.\n");
		}
	}
	
	// Store the ID
	*id = buffer_used;
}

/* Bind buffer */
void gfxBindBuffer(unsigned int id)
{
	// Means activate buffer specified by id
	buffer_active = id;
}

/* Allocate buffer data */
void gfxBufferData(enum buffer_type type, size_t items_size, void* data)
{	
	// Error catching
	if(buffer_active == 0)
	{
		gfxSetErrorMessage("Error: Attemping to create buffer on NULL buffer.\n");
		return;
	}
	
	// Allocate memory for buffer and store variables, or use data pointed by argument
	size_t item_size = sizeof(float) * 4;
	
	buffers[buffer_active].item_size = item_size;
	buffers[buffer_active].items_count = items_size / item_size;
	buffers[buffer_active].type = type;

	if(data == NULL)
	{
		buffers[buffer_active].data = gfx_alloc(sizeof(unsigned char) * items_size, 16);
	}
	else
	{
		buffers[buffer_active].data = data;
	}
}

/* Delete the buffer (deletes the data also) */
void gfxDeleteBuffer(unsigned int id)
{
	gfx_free(buffers[id].data);
	buffers[id].data = NULL;
}

/* Mapping the buffer */
void* gfxMapBuffer()
{
	// Gives us pointer to memory
	if(buffer_active == 0)
	{
		gfxSetErrorMessage("Error: Attemping to map NULL buffer.\n");
		return NULL;
	}
	
	if(buffers[buffer_active].data == NULL)
	{
		gfxSetErrorMessage("Error: Attemping to map buffer with NULL data.\n");
		return NULL;
	}
	
	return buffers[buffer_active].data;
}

/* Unmapping buffer */
bool gfxUnmapBuffer()
{
	// Actually nothing needed here, as we're not threaded (for more info read the tutorial)
	return true;
}

/* Draw arrays (calls rasterization & shading device) */
void gfxDrawArrays(enum draw_type draw, size_t offset, unsigned int count)
{
	_dev_pipeline(draw, offset, count);
}
