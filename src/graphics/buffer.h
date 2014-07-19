#ifndef _BUFFER_H
#define _BUFFER_H

#include "gfx.h"

/* Buffer structure */
struct buffer_t
{
	size_t item_size;					// Buffer single item size
	unsigned int items_count;	// Total items count in buffer
	enum buffer_type type;		// Buffer type (currently only GFX_ARRAY_BUFFER)
	void* data;								// Buffer data
};

// Externs, see buffer.c for more information
extern struct buffer_t* buffers;
extern unsigned int buffer_count;
extern unsigned int buffer_used;
extern unsigned int buffer_active;

#endif
