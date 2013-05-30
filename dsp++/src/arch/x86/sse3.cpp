#include <dsp++/platform.h>

#ifdef DSP_ARCH_FAMILY_X86

#include <dsp++/simd.h>
#include "sse.h"
#include "sse_utils.h"

#include <pmmintrin.h>

//! @brief Dot product using SSE3 instruction set.
float dsp::simd::detail::x86_sse3_dotf(const float* x, const float* b, size_t N)
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

#endif // DSP_ARCH_FAMILY_X86
