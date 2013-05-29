/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/vectmath.h>
#include <dsp++/simd.h>
#include <cstring>

#ifdef DSP_ARCH_FAMILY_X86
# include <xmmintrin.h> 	// SSE intrinsics
# include <pmmintrin.h>	// SSE3 intrinsics
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
	// TODO iterations for a_ and b_ should be split as they can have different lengths
	float res = 0.f;
	__m128 c0, c1, c2, c3, x0, x1, x2, x3;
	size_t n = N / 16;
	for (size_t i = 0; i < n; ++i, a += 16, w += 16, b += 16) {
		c0 = _mm_load_ps(a); 			// c = a
		c1 = _mm_load_ps(a + 4);
		c2 = _mm_load_ps(a + 8);
		c3 = _mm_load_ps(a + 12);
		x0 = _mm_loadu_ps(w); 			// x = w
		x1 = _mm_loadu_ps(w + 4);
		x2 = _mm_loadu_ps(w + 8);
		x3 = _mm_loadu_ps(w + 12);
		c0 = _mm_mul_ps(c0, x0);		// c *= x (c = w * a)
		c1 = _mm_mul_ps(c1, x1);
		c2 = _mm_mul_ps(c2, x2);
		c3 = _mm_mul_ps(c3, x3);
		x0 = _mm_sub_ps(x0, c0);		// x -= c (x = w - w * a)
		x1 = _mm_sub_ps(x1, c1);
		x2 = _mm_sub_ps(x2, c2);
		x3 = _mm_sub_ps(x3, c3);
		_mm_storeu_ps(w, x0);			// w = x
		_mm_storeu_ps(w + 4, x1);
		_mm_storeu_ps(w + 8, x2);
		_mm_storeu_ps(w + 12, x3);

		c0 = _mm_load_ps(b);			// c = b
		c1 = _mm_load_ps(b + 4);
		c2 = _mm_load_ps(b + 8);
		c3 = _mm_load_ps(b + 12);
		c0 = _mm_mul_ps(c0, x0);		// c *= x (c = w * b)
		c1 = _mm_mul_ps(c1, x1);
		c2 = _mm_mul_ps(c2, x2);
		c3 = _mm_mul_ps(c3, x3);

		x0 = _mm_add_ps(x0, x1);		// sum(x)
		x2 = _mm_add_ps(x2, x3);
		x0 = _mm_add_ps(x0, x2);

		x1 = _mm_add_ps(x0, _mm_movehl_ps(x0, x0));  		// horizontal sum(x0)
		x0 = _mm_add_ss(x1, _mm_shuffle_ps(x1, x1, 1));
		res += _mm_cvtss_f32(x0);
	}
	n = (N % 16) / 4;
	for (size_t i = 0; i < n; ++i, a += 4, w += 4, b += 4) {
		c0 = _mm_load_ps(a); 			// c = a
		x0 = _mm_loadu_ps(w); 			// x = w
		c0 = _mm_mul_ps(c0, x0);		// c *= x (c = w * a)
		x0 = _mm_sub_ps(x0, c0);		// x -= c (x = w - w * a)
		_mm_storeu_ps(w, x0);			// w = x
		c0 = _mm_load_ps(b);			// c = b
		c0 = _mm_mul_ps(c0, x0);		// c *= x (c = w * b)

		x1 = _mm_add_ps(x0, _mm_movehl_ps(x0, x0));  		// horizontal sum(x0)
		x0 = _mm_add_ss(x1, _mm_shuffle_ps(x1, x1, 1));
		res += _mm_cvtss_f32(x0);
	}
	return res;
}

#endif
}

DSPXX_API float dsp::simd::filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	if (false) ;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		return filter_df2_sse_(w, b, M, a, N);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::filter_sample_df2(w, b, M, a, N);
}
