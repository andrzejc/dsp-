/*!
 * @file vectmath.cpp
 * @brief Implementation of various vector operations.
 */
#include <dsp++/platform.h>
#include <dsp++/simd.h>
#include <dsp++/vectmath.h>
#include <cmath>
#include <cstring>

#ifdef DSP_ARCH_FAMILY_X86
#include "sse.h"
#endif // DSP_ARCH_FAMILY_X86

namespace {

#ifdef DSP_ARCH_FAMILY_X86

//! @brief Piecewise vector multiplication using SSE instructions
static inline void mulf_sse_(float* res, const float* x, const float* b, size_t N)
{
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;
	size_t n = N / 16;
	for (size_t i = 0; i < n; ++i, b += 16, x += 16, res += 16) {
		SSE_LOAD16(b, b);
		SSE_LOAD16(x, x);
		SSE_MUL16(x, x, b);
		SSE_STORE16(res, x);
	}
	n = (N % 16) / 4;
	for (size_t i = 0; i < n; ++i, b += 4, x += 4, res += 4) {
		b0 = _mm_load_ps(b);
		x0 = _mm_load_ps(x);
		x0 = _mm_mul_ps(x0, b0);
		_mm_store_ps(res, x0);
	}
}

//! @brief Piecewise complex vector multiplication using SSE instructions
static inline void mulcf_sse_(float* res, const float* a, const float* b, size_t len)
{
	__m128 x0, x1, x2, x3, x4;
	float DSP_ALIGNED(16) mul[] = {-1.0f, 1.0f, -1.0f, 1.0f};
	size_t n = len / 2; // each complex has 2 floats, so divide by 2 not 4
	x4 = _mm_load_ps(mul);
	for (size_t i = 0; i < n; ++i, a += 4, b += 4, res += 4) {
		x0 = _mm_load_ps(a);
		x1 = _mm_load_ps(b);
		x2 = x1;
		x3 = x0;
		x2 = _mm_shuffle_ps(x2, x1, 0xA0);
		x1 = _mm_shuffle_ps(x1, x1, 0xF5);
		x3 = _mm_shuffle_ps(x3, x0, 0xB1);
		x0 = _mm_mul_ps(x0, x2);
		x3 = _mm_mul_ps(x3, x1);
		x3 = _mm_mul_ps(x3, x4);
		x0 = _mm_add_ps(x0, x3);
		_mm_store_ps(res, x0);
	}
}

// TODO implement complex mul/div using SSE3 instructions according to Ex 6-9

//! @brief Dot product of complex vectors using SSE instructions
static inline std::complex<float> dotcf_sse_(const float* a, const float* b, size_t len)
{
	__m128 x0, x1, x2, x3, x4, x5;
	float DSP_ALIGNED(16) mul[] = {-1.0f, 1.0f, -1.0f, 1.0f};
	size_t n = len / 2; // each complex has 2 floats, so divide by 2 not 4
	x5 = _mm_set1_ps(0.f); // write zeros to result
	x4 = _mm_load_ps(mul);
	for (size_t i = 0; i < n; ++i, a += 4, b += 4) {
		x0 = _mm_load_ps(a);
		x1 = _mm_load_ps(b);
		x2 = x1;
		x3 = x0;
		x2 = _mm_shuffle_ps(x2, x1, 0xA0);
		x1 = _mm_shuffle_ps(x1, x1, 0xF5);
		x3 = _mm_shuffle_ps(x3, x0, 0xB1);
		x0 = _mm_mul_ps(x0, x2);
		x3 = _mm_mul_ps(x3, x1);
		x3 = _mm_mul_ps(x3, x4);
		x0 = _mm_add_ps(x0, x3);
		x5 = _mm_add_ps(x5, x0);
	}
	// x5 now has 2 complex numbers which need to be added
	x0 = _mm_movehl_ps(x0, x5);
	x0 = _mm_add_ps(x0, x5);
	_mm_store_ps(mul, x0);
	return *reinterpret_cast<std::complex<float>*>(mul);
}

//!@brief Dot product using SSE instruction set.
static inline float dotf_sse_(const float* x, const float* b, size_t N)
{
	float res = 0.f;
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;
	size_t n = N / 16;
	for (size_t i = 0; i < n; ++i, b += 16, x += 16) {
		SSE_LOAD16(b, b);
		SSE_LOAD16(x, x);
		SSE_MUL16(x, x, b);
		SSE_HSUM16(x0, x);
		res += _mm_cvtss_f32(x0);
	}
	n = (N % 16) / 4;
	for (size_t i = 0; i < n; ++i, b += 4, x += 4) {
		b0 = _mm_load_ps(b);
		x0 = _mm_load_ps(x);
		x0 = _mm_mul_ps(x0, b0);
		SSE_HSUM(x0, x0, x1);
		res += _mm_cvtss_f32(x0);
	}
	return res;
}

//! @brief Dot product using SSE3 instruction set.
static inline float dotf_sse3_(const float* x, const float* b, size_t N)
{
	float res = 0.f;
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;
	size_t n = N / 16;
	for (size_t i = 0; i < n; ++i, x += 16, b += 16) {
		SSE_LOAD16(b, b);
		SSE_LOAD16(x, x);
		SSE_MUL16(x, x, b);
		SSE3_HSUM16(x0, x);
		res += _mm_cvtss_f32(x0);
	}
	n = (N % 16) / 4;
	for (size_t i = 0; i < n; ++i, b += 4, x += 4) {
		b0 = _mm_load_ps(b);
		x0 = _mm_load_ps(x);
		x0 = _mm_mul_ps(x0, b0);
		SSE3_HSUM(x0, x0);
		res += _mm_cvtss_f32(x0);
	}
	return res;
}

//! @brief Dot product using SSE4.1 instruction set.
static inline float dotf_sse41_(const float* x, const float* b, size_t N)
{
	float res = 0.f;
	__m128 b0, b1, b2, b3, x0, x1, x2, x3;
	size_t n = N / 16;
	for (size_t i = 0; i < n; ++i, x += 16, b += 16) {
		SSE_LOAD16(b, b);
		SSE_LOAD16(x, x);
		SSE41_DP16(x, x, b, 0xf1); 		// 0xf1 = 11110001b, all multiplies, put result in lower dword
		SSE_SUM16(x0, x);
		res += _mm_cvtss_f32(x0);
	}
	n = (N % 16) / 4;
	for (size_t i = 0; i < n; ++i, b += 4, x += 4) {
		b0 = _mm_load_ps(b);
		x0 = _mm_load_ps(x);
		x0 = _mm_dp_ps(x0, b0, 0xf1);
		res += _mm_cvtss_f32(x0);
	}
	return res;
}

#endif // DSP_ARCH_FAMILY_X86

} // end of anonymous namespace 

DSPXX_API float dsp::simd::dot(const float* v0, const float* v1, size_t len)
{
	if (false) 	;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse41)
		return dotf_sse41_(v0, v1, len);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse3)
		return dotf_sse3_(v0, v1, len);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		return dotf_sse_(v0, v1, len);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::dot(v0, v1, len);
}

DSPXX_API std::complex<float> dsp::simd::dot(const std::complex<float>* a, const std::complex<float>* b, size_t len)
{
	if (false) 	;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		return dotcf_sse_(reinterpret_cast<const float*>(a), reinterpret_cast<const float*>(b), len);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::dot(a, b, len);

}

DSPXX_API void dsp::simd::mul(float* res, const float* a, const float* b, size_t len)
{
	if (false) ;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		mulf_sse_(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::mul(res, a, b, len);
}

DSPXX_API void dsp::simd::mul(std::complex<float>* res, const std::complex<float>* a, const std::complex<float>* b, size_t len)
{
	if (false) ;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		mulcf_sse_(reinterpret_cast<float*>(res), reinterpret_cast<const float*>(a), reinterpret_cast<const float*>(b), len);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::mul(res, a, b, len);
}
