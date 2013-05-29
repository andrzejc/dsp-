/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/vectmath.h>
#include <dsp++/simd.h>
#include <cstring>

#ifdef DSP_ARCH_FAMILY_X86
# include "sse.h"
#endif // DSP_ARCH_FAMILY_X86

using namespace dsp;

namespace {
#ifdef DSP_ARCH_FAMILY_X86

/*!
 * @brief Implementation of Direct-Form II FIR/IIR filter using SSE instruction set.
 * @param w Delay line buffer of length max(M, N), needs not to be aligned (we use unaligned reads here).
 * @param b FIR filter coefficients vector, must be aligned and padded (M)
 * @param M Padded length of b vector.
 * @param a IIR filter coefficients vector, must be aligned and padded, a[0] must be set to 0 for efficiency reasons (N)
 * @param N Padded length of a vector.
 * @note M and N must include padding.
 * @note a[0] must be set to 0, although it is assumed to be 1.
 * @return filtered sample.
 */
static inline float filter_df2_sse_(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	float ardot = 0.f, madot = 0, *ws = w, b0 = *b;
	__m128 c0, c1, c2, c3, x0, x1, x2, x3;
	size_t L = std::min(N, M);
	size_t n = L / 16;
	// Simultaneous calculation of both AR- and MA- component dot products, first in 16-, then 4- element chunks
	for (size_t i = 0; i < n; ++i, a += 16, w += 16, b += 16) {
		SSE_LOADU16(x, w);				// x = w
		SSE_LOAD16(c, a);				// c = a
		SSE_MUL16(c, c, x); 			// c *= x (c = w * a)
		SSE_HSUM16(c0, c);
		ardot += _mm_cvtss_f32(c0);

		SSE_LOAD16(c, b);				// c = b
		SSE_MUL16(c, c, x);				// c *= x (c = w * b)
		SSE_HSUM16(c0, c); 				// c0 = sum(c)
		madot += _mm_cvtss_f32(c0);
	}
	n = (L % 16) / 4;
	for (size_t i = 0; i < n; ++i, a += 4, w += 4, b += 4) {
		x0 = _mm_loadu_ps(w);
		c0 = _mm_load_ps(a);
		c0 = _mm_mul_ps(c0, x0);
		SSE_HSUM(c0, c0, c1);
		ardot += _mm_cvtss_f32(c0);

		c0 = _mm_load_ps(b);
		c0 = _mm_mul_ps(c0, x0);
		SSE_HSUM(c0, c0, c1);
		madot += _mm_cvtss_f32(c0);
	}
	L = std::max(N, M) - L;
	n = L / 16;
	if (N > M) {
		// Calculate only the remaining AR-component product
		for (size_t i = 0; i < n; ++i, a += 16, w += 16) {
			SSE_LOADU16(x, w);				// x = w
			SSE_LOAD16(c, a);				// c = a
			SSE_MUL16(c, c, x); 			// c *= x (c = w * a)
			SSE_HSUM16(c0, c);
			ardot += _mm_cvtss_f32(c0);
		}
		n = (L % 16) / 4;
		for (size_t i = 0; i < n; ++i, a += 4, w += 4) {
			x0 = _mm_loadu_ps(w);
			c0 = _mm_load_ps(a);
			c0 = _mm_mul_ps(c0, x0);
			SSE_HSUM(c0, c0, c1);
			ardot += _mm_cvtss_f32(c0);
		}
	}
	else {
		// Calculate only the remaining MA-component
		for (size_t i = 0; i < n; ++i, b += 16, w += 16) {
			SSE_LOADU16(x, w);				// x = w
			SSE_LOAD16(c, b);				// c = b
			SSE_MUL16(c, c, x);				// c *= x (x = w * b)
			SSE_HSUM16(c0, c); 				// c0 = sum(c)
			madot += _mm_cvtss_f32(c0);
		}
		n = (L % 16) / 4;
		for (size_t i = 0; i < n; ++i, b += 4, w += 4) {
			x0 = _mm_loadu_ps(w);
			c0 = _mm_load_ps(b);
			c0 = _mm_mul_ps(c0, x0);
			SSE_HSUM(c0, c0, c1);
			madot += _mm_cvtss_f32(c0);
		}
	}
	// In canonical equation, AR product should be computed first and subtracted from w[0] (which is in fact input sample)
	// before calculating MA product. Since we computed them simultaneously, MA product must be corrected now, taking into
	// account that w[0] was multiplied by b[0].
	madot -= ardot * b0;
	// Update w[0] as well.
	*ws -= ardot;
	return madot;
}

static inline float filter_df2_sse41_(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	float ardot = 0.f, madot = 0, *ws = w, b0 = *b;
	__m128 c0, c1, c2, c3, x0, x1, x2, x3;
	size_t L = std::min(N, M);
	size_t n = L / 16;
	// Simultaneous calculation of both AR- and MA- component dot products, first in 16-, then 4- element chunks
	for (size_t i = 0; i < n; ++i, a += 16, w += 16, b += 16) {
		SSE_LOADU16(x, w);
		SSE_LOAD16(c, a);
		SSE41_DP16(c, c, x, 0xf1);
		SSE_SUM16(c0, c);
		ardot += _mm_cvtss_f32(c0);

		SSE_LOAD16(c, b);
		SSE41_DP16(c, c, x, 0xf1);
		SSE_SUM16(c0, c);
		madot += _mm_cvtss_f32(c0);
	}
	n = (L % 16) / 4;
	for (size_t i = 0; i < n; ++i, a += 4, w += 4, b += 4) {
		x0 = _mm_loadu_ps(w);
		c0 = _mm_load_ps(a);
		c0 = _mm_dp_ps(c0, x0, 0xf1);
		ardot += _mm_cvtss_f32(c0);

		c0 = _mm_load_ps(b);
		c0 = _mm_dp_ps(c0, x0, 0xf1);
		madot += _mm_cvtss_f32(c0);
	}
	L = std::max(N, M) - L;
	n = L / 16;
	if (N > M) {
		// Calculate only the remaining AR-component product
		for (size_t i = 0; i < n; ++i, a += 16, w += 16) {
			SSE_LOADU16(x, w);
			SSE_LOAD16(c, a);
			SSE41_DP16(c, c, x, 0xf1);
			SSE_SUM16(c0, c);
			ardot += _mm_cvtss_f32(c0);
		}
		n = (L % 16) / 4;
		for (size_t i = 0; i < n; ++i, a += 4, w += 4) {
			x0 = _mm_loadu_ps(w);
			c0 = _mm_load_ps(a);
			c0 = _mm_dp_ps(c0, x0, 0xf1);
			ardot += _mm_cvtss_f32(c0);
		}
	}
	else {
		// Calculate only the remaining MA-component
		for (size_t i = 0; i < n; ++i, b += 16, w += 16) {
			SSE_LOADU16(x, w);
			SSE_LOAD16(c, b);
			SSE41_DP16(c, c, x, 0xf1);
			SSE_SUM16(c0, c);
			madot += _mm_cvtss_f32(c0);
		}
		n = (L % 16) / 4;
		for (size_t i = 0; i < n; ++i, b += 4, w += 4) {
			x0 = _mm_loadu_ps(w);
			c0 = _mm_load_ps(b);
			c0 = _mm_dp_ps(c0, x0, 0xf1);
			madot += _mm_cvtss_f32(c0);
		}
	}
	// In canonical equation, AR product should be computed first and subtracted from w[0] (which is in fact input sample)
	// before calculating MA product. Since we computed them simultaneously, MA product must be corrected now, taking into
	// account that w[0] was multiplied by b[0].
	madot -= ardot * b0;
	// Update w[0] as well.
	*ws -= ardot;
	return madot;
}

#endif
}

DSPXX_API float dsp::simd::filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	if (false) ;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse41)
		return filter_df2_sse41_(w, b, M, a, N);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		return filter_df2_sse_(w, b, M, a, N);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::filter_sample_df2(w, b, M, a, N);
}
