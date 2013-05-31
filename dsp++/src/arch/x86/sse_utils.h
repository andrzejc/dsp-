/*!
 * @file sse_utils.h
 * @brief SSE support header
 */

#ifndef DSP_SSE_UTILS_H_INCLUDED
#define DSP_SSE_UTILS_H_INCLUDED
#pragma once

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

#define SSE41_DP16(to, a, b, mask) \
	to ## 0 = _mm_dp_ps(a ## 0, b ## 0, mask); \
	to ## 1 = _mm_dp_ps(a ## 1, b ## 1, mask); \
	to ## 2 = _mm_dp_ps(a ## 2, b ## 2, mask); \
	to ## 3 = _mm_dp_ps(a ## 3, b ## 3, mask)

// binary float op(vector,vector) = vector
#define SSE_FVVV(name, op) \
void name(float* res, const float* x, const float* b, size_t N) {	 		\
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;									\
	size_t n = N / 16;														\
	for (size_t i = 0; i < n; ++i, b += 16, x += 16, res += 16) {			\
		SSE_LOAD16(b, b);													\
		SSE_LOAD16(x, x);													\
		SSE_BINARY16_REG_REG(x, x, b, op);									\
		SSE_STORE16(res, x);												\
	}																		\
	n = (N % 16) / 4;														\
	for (size_t i = 0; i < n; ++i, b += 4, x += 4, res += 4) {				\
		b0 = _mm_load_ps(b);												\
		x0 = _mm_load_ps(x);												\
		x0 = _mm_ ## op(x0, b0);											\
		_mm_store_ps(res, x0);												\
	}																		\
}

// binary float sum(op(vector,vector)) = scalar
#define SSE_SUM_FVVS(name, op) \
float name(const float* x, const float* b, size_t N) {	 					\
	register float res = 0.f;												\
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;									\
	size_t n = N / 16;														\
	for (size_t i = 0; i < n; ++i, b += 16, x += 16, res += 16) {			\
		SSE_LOAD16(b, b);													\
		SSE_LOAD16(x, x);													\
		SSE_BINARY16_REG_REG(x, x, b, op);									\
		SSE_HSUM16(x0, x);													\
		res += _mm_cvtss_f32(x0);											\
	}																		\
	n = (N % 16) / 4;														\
	for (size_t i = 0; i < n; ++i, b += 4, x += 4, res += 4) {				\
		b0 = _mm_load_ps(b);												\
		x0 = _mm_load_ps(x);												\
		x0 = _mm_ ## op(x0, b0);											\
		SSE_HSUM(x0, x0, x1);												\
		res += _mm_cvtss_f32(x0);											\
	}																		\
	return res;																\
}


// binary float op(vector,scalar) = vector
#define SSE_FVSV(name, op) \
void name(float* res, const float* x, float s, size_t N) {					\
	__m128 x0, x1, x2, x3, x4, x5, x6, s0;									\
	size_t n = N / 28;														\
	s0 = _mm_load1_ps(&s);													\
	for (size_t i = 0; i < n; ++i, x += 28, res += 28) {					\
		x0 = _mm_load_ps(x);												\
		x1 = _mm_load_ps(x + 4);											\
		x2 = _mm_load_ps(x + 8);											\
		x3 = _mm_load_ps(x + 12);											\
		x4 = _mm_load_ps(x + 16);											\
		x5 = _mm_load_ps(x + 20);											\
		x6 = _mm_load_ps(x + 24);											\
		x0 = _mm_## op(x0, s0);												\
		x1 = _mm_## op(x1, s0);												\
		x2 = _mm_## op(x2, s0);												\
		x3 = _mm_## op(x3, s0);												\
		x4 = _mm_## op(x4, s0);												\
		x5 = _mm_## op(x5, s0);												\
		x6 = _mm_## op(x6, s0);												\
		_mm_store_ps(res, x0);												\
		_mm_store_ps(res + 4, x1);											\
		_mm_store_ps(res + 8, x2);											\
		_mm_store_ps(res + 12, x3);											\
		_mm_store_ps(res + 16, x4);											\
		_mm_store_ps(res + 20, x5);											\
		_mm_store_ps(res + 24, x6);											\
	}																		\
	n = (N % 28) / 4;														\
	for (size_t i = 0; i < n; ++i, x += 4, res += 4) {						\
		x0 = _mm_load_ps(x);												\
		x0 = _mm_## op(x0, s0);												\
		_mm_store_ps(res, x0);												\
	}																		\
}

// unary float op(vector) = vector
#define SSE_FVV(name, op)	\
void name(float* res, const float* x, size_t N) {							\
	__m128 x0, x1, x2, x3, x4, x5, x6, x7;									\
	size_t n = N / 32;														\
	for (size_t i = 0; i < n; ++i, x += 32, res += 32) {					\
		x0 = _mm_load_ps(x);												\
		x1 = _mm_load_ps(x + 4);											\
		x2 = _mm_load_ps(x + 8);											\
		x3 = _mm_load_ps(x + 12);											\
		x4 = _mm_load_ps(x + 16);											\
		x5 = _mm_load_ps(x + 20);											\
		x6 = _mm_load_ps(x + 24);											\
		x7 = _mm_load_ps(x + 28);											\
		x0 = _mm_ ## op(x0);												\
		x1 = _mm_ ## op(x1);												\
		x2 = _mm_ ## op(x2);												\
		x3 = _mm_ ## op(x3);												\
		x4 = _mm_ ## op(x4);												\
		x5 = _mm_ ## op(x5);												\
		x6 = _mm_ ## op(x6);												\
		x7 = _mm_ ## op(x7);												\
		_mm_store_ps(res, x0);												\
		_mm_store_ps(res + 4, x1);											\
		_mm_store_ps(res + 8, x2);											\
		_mm_store_ps(res + 12, x3);											\
		_mm_store_ps(res + 16, x4);											\
		_mm_store_ps(res + 20, x5);											\
		_mm_store_ps(res + 24, x6);											\
		_mm_store_ps(res + 28, x7);											\
	}																		\
	n = (N % 32) / 4;														\
	for (size_t i = 0; i < n; ++i, x += 4, res += 4) {						\
		x0 = _mm_load_ps(x);												\
		x0 = _mm_ ## op(x0);												\
		_mm_store_ps(res, x0);												\
	}																		\
}


#endif /* DSP_SSE_UTILS_H_INCLUDED */
