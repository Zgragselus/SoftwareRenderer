#ifndef _MEMORY_H
#define _MEMORY_H

#include <stdlib.h>
#include <malloc.h>

// Don't try to find magic in this code, this is just plain simple allocation/free for aligned memory - because aligned memory 
// is equal to fast memory access when it comes to SSE (and we WILL use SSE later on).

/* Allocation */
// Just pass number of bytes to allocate and alignment (note we'll mostly use 16-byte alignment)
static inline void* gfx_alloc(size_t size, size_t align)
{
	// Just in case if we don't need any alignment - use malloc
	if(align == 1)
	{
		return malloc(size);
	}
	
	// The alignment must be 2^n and higher than sizeof(void*) - native on defined architecture
	if(!(align & (align - 1)) && align < sizeof(void*))
	{
		align = sizeof(void*);
	}
	
	// Aligned allocation - system specific
	void* allocated;
	
#ifdef _WIN32
	allocated = _aligned_malloc(size, align);
#elif defined __MINGW32__
	allocated = __mingw_aligned_malloc(size, align);
#else
	if(posix_memalign(&allocated, align, size) != 0)
	{
		return 0;
	}
#endif

	return allocated;
}

/* Simple memory free */
static inline void gfx_free(void* p)
{
	// Just system specific call for free of aligned memory (note the POSIX uses standard free())

#ifdef _WIN32
	_aligned_free(p);
#elif defined __MINGW32__
	__mingw_aligned_free(p);
#else
	free(p);
#endif
}

#endif
