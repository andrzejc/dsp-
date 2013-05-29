/*!
 * @file sse.h
 * @brief SSE support header
 */

#ifndef DSP_SSE_H_INCLUDED
#define DSP_SSE_H_INCLUDED
#pragma once

# include <xmmintrin.h> 	// SSE intrinsics
# include <pmmintrin.h>		// SSE3 intrinsics

#define SSE_UNARY16_REG_MEM(to, from, oper) \
	to ## 0 = _mm_ ## oper(from); \
	to ## 1 = _mm_ ## oper(from + 4); \
	to ## 2 = _mm_ ## oper(from + 8); \
	to ## 3 = _mm_ ## oper(from + 12)

#define SSE_BINARY16_REG_REG(to, a, b, oper) \
	to ## 0 = _mm_ ## oper(a ## 0, b ## 0); \
	to ## 1 = _mm_ ## oper(a ## 1, b ## 1); \
	to ## 2 = _mm_ ## oper(a ## 2, b ## 2); \
	to ## 3 = _mm_ ## oper(a ## 3, b ## 3)

#define SSE_BINARY16_MEM_REG(to, from, oper) \
	_mm_ ## oper(to, from ## 0); \
	_mm_ ## oper(to + 4, from ## 1); \
	_mm_ ## oper(to + 8, from ## 2); \
	_mm_ ## oper(to + 12, from ## 3)


#define SSE_LOADU16(to, from) \
	SSE_UNARY16_REG_MEM(to, from, loadu_ps)

#define SSE_LOAD16(to, from) \
	SSE_UNARY16_REG_MEM(to, from, load_ps)

#define SSE_MUL16(to, a, b) \
	SSE_BINARY16_REG_REG(to, a, b, mul_ps)

#define SSE_SUB16(to, a, b) \
	SSE_BINARY16_REG_REG(to, a, b, sub_ps)

#define SSE_ADD16(to, a, b) \
	SSE_BINARY16_REG_REG(to, a, b, add_ps)

#define SSE_STOREU16(to, from) \
	SSE_BINARY16_MEM_REG(to, from, storeu_ps)

#define SSE_STORE16(to, from) \
	SSE_BINARY16_MEM_REG(to, from, store_ps)

#define SSE_HSUM(to, from, slack) \
	slack = _mm_add_ps(from, _mm_movehl_ps(from, from)); \
	to = _mm_add_ss(slack, _mm_shuffle_ps(slack, slack, 1))

#define SSE3_HSUM(to, from) \
	to = _mm_hadd_ps(from, from); \
	to = _mm_hadd_ps(to, to)

#define SSE_SUM16(to, from) \
	to = _mm_add_ps(from ## 0, from ## 1); \
	from ## 1 = _mm_add_ps(from ## 2, from ## 3); \
	to = _mm_add_ps(to, from ## 1)

#define SSE_HSUM16(to, from) \
	SSE_SUM16(from ## 0, from); \
	SSE_HSUM(to, from ## 0, from ## 1)

#define SSE3_HSUM16(to, from) \
	SSE_SUM16(from ## 0, from); \
	SSE3_HSUM(to, from ## 0)

#endif /* DSP_SSE_H_INCLUDED */
