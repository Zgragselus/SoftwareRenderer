#ifndef _MAT4_H
#define _MAT4_H

#include <xmmintrin.h>
#include <emmintrin.h>

#include "float4.h"

/* Matrix packed 4-row structure */
// This is needed, because we can't use anonymous structures in C99
typedef struct __attribute__((aligned(16)))
{
	__m128 m1;
	__m128 m2;
	__m128 m3;
	__m128 m4;
}
mat4_f;

/* Mat4 structure */
// Just a union of our packed 4-row structure and 4 item SSE __m128 array C data type
typedef union __attribute__((aligned(16))) 
{
	mat4_f fp;
	__m128 m[4];
}
mat4;

/* Matrix-matrix multiplication */
inline mat4 mul_mat_mat(mat4 m0, mat4 m1)
{
	mat4 result;
	
	result.m[0] = _mm_add_ps(_mm_add_ps(		
									_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m0.fp.m1, m0.fp.m1, 0x00), m1.fp.m1),		
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m1, m0.fp.m1, 0x55), m1.fp.m2)),					
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m1, m0.fp.m1, 0xAA), m1.fp.m3)),						
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m1, m0.fp.m1, 0xFF), m1.fp.m4));

	result.m[1] = _mm_add_ps(_mm_add_ps(							
									_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m0.fp.m2, m0.fp.m2, 0x00), m1.fp.m1),	
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m2, m0.fp.m2, 0x55), m1.fp.m2)),				
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m2, m0.fp.m2, 0xAA), m1.fp.m3)),				
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m2, m0.fp.m2, 0xFF), m1.fp.m4));

	result.m[2] = _mm_add_ps(_mm_add_ps(							
									_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m0.fp.m3, m0.fp.m3, 0x00), m1.fp.m1),		
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m3, m0.fp.m3, 0x55), m1.fp.m2)),				
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m3, m0.fp.m3, 0xAA), m1.fp.m3)),			
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m3, m0.fp.m3, 0xFF), m1.fp.m4));
									
	result.m[3] = _mm_add_ps(_mm_add_ps(								
									_mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(m0.fp.m4, m0.fp.m4, 0x00), m1.fp.m1),	
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m4, m0.fp.m4, 0x55), m1.fp.m2)),				
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m4, m0.fp.m4, 0xAA), m1.fp.m3)),				
									_mm_mul_ps(_mm_shuffle_ps(m0.fp.m4, m0.fp.m4, 0xFF), m1.fp.m4));
									
	return result;
}

/* Matrix-Vector multiplication */
inline float4 mul_mat_vec(mat4 m, float4 v)
{
	float4 result;
	
	result.xmm = _mm_add_ps(_mm_add_ps(_mm_mul_ps(m.fp.m1, _mm_shuffle_ps(v.xmm, v.xmm, 0x00)),
											_mm_mul_ps(m.fp.m2, _mm_shuffle_ps(v.xmm, v.xmm, 0x55))),
									_mm_add_ps(_mm_mul_ps(m.fp.m3, _mm_shuffle_ps(v.xmm, v.xmm, 0xaa)),
											_mm_mul_ps(m.fp.m4, _mm_shuffle_ps(v.xmm, v.xmm, 0xff))));
											
	return result;
}

/* Projection matrix setup */
inline mat4 projection(float fov, float aspect, float n, float f)
{
	float _asp = 1.0f / aspect;
	float _fov = 1.0f / tanf(fov * 0.5f * 3.14159265f / 180.0f);

	mat4 r;
	r.fp.m1 = _mm_setr_ps(_fov * _asp, 0.0f, 0.0f, 0.0f);
	r.fp.m2 = _mm_setr_ps(0.0f, _fov, 0.0f, 0.0f);
	r.fp.m3 = _mm_setr_ps(0.0f, 0.0f, -(f + n) / (f - n), -(2.0f * f * n) / (f - n));
	r.fp.m4 = _mm_setr_ps(0.0f, 0.0f, -1.0f, 1.0f);
	
	return r;
}

#endif
