#include <dsp++/platform.h>

#ifdef DSP_ARCH_FAMILY_X86

#include <dsp++/simd.h>
#include "sse.h"
#include "sse_utils.h"

#include <smmintrin.h>

//! @brief Dot product using SSE3 instruction set.
float dsp::simd::detail::x86_sse41_dotf(const float* x, const float* b, size_t N)
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


float dsp::simd::detail::x86_sse41_filter_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
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

#endif // DSP_ARCH_FAMILY_X86
