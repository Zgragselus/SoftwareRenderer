#ifndef _FLOAT4_H
#define _FLOAT4_H

#include <xmmintrin.h>
#include <emmintrin.h>

/* Simple floating point packed-4D-vector structure */
// This is needed, because we can't use anonymous structures in C99
#ifndef WINDOWS
typedef struct __attribute__((aligned(16)))
#else
typedef struct __declspec(align(16))
#endif
{
	float x, y, z, w;
}
float4_f;

/* Float4 structure */
// Just a union of our packed-4D-vector structure and SSE __m128 C data type
#ifndef WINDOWS
typedef union __attribute__((aligned(16)))
#else
typedef union __declspec(align(16))
#endif
{
	float4_f fp;
	__m128 xmm;
}
float4;

/* Constructor */
inline float4 float4_from_m128(__m128 value)
{
	float4 f;
	f.xmm = value;
	return f;
}

/* Dot product */
inline float dot(float4* a, float4* b)
{
	__m128 l = _mm_mul_ps(a->xmm, b->xmm);
	l = _mm_add_ps(l, _mm_shuffle_ps(l, l, 0x4E));
	return _mm_cvtss_f32(_mm_add_ss(l, _mm_shuffle_ps(l, l, 0x11)));
}

/* Cross product */
inline float4 cross(float4* a, float4* b)
{
	const __m128 l  = _mm_mul_ps(_mm_shuffle_ps(a->xmm, a->xmm, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b->xmm, b->xmm, _MM_SHUFFLE(3, 1, 0, 2)));
	const __m128 r = _mm_mul_ps(_mm_shuffle_ps(a->xmm, a->xmm, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b->xmm, b->xmm, _MM_SHUFFLE(3, 0, 2, 1)));
	return float4_from_m128(_mm_sub_ps(l, r));
} 

#endif
