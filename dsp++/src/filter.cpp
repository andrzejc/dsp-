/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/platform.h>
#include <dsp++/simd.h>

#ifdef DSP_ARCH_FAMILY_X86
#include <immintrin.h>
#include <xmmintrin.h>
#endif // DSP_ARCH_FAMILY_X86

using namespace dsp;

#if 0
#ifdef DSP_ARCH_FAMILY_X86
static void basic_fir(const float* x, float* y, size_t L, float* w, const float* b, size_t N)
{
	if (dsp::simd::feat_x86_sse & dsp::simd::features()) {
		__m128 xmm0 = _mm_load_ps(b);
		__m128 xmm1 = _mm_load_ps(b + 4);
		__m128 xmm2 = _mm_load_ps(b + 8);
		__m128 xmm3 = _mm_load_ps(b + 12);

	}
}
#endif // DSP_ARCH_FAMILY_X86

#endif


