/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/simd.h>
#include <cstring>

#include "arch/x86/sse.h"

using namespace dsp;

float dsp::simd::filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	if (false) ;
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse41)
		return dsp::simd::detail::x86_sse41_filter_df2(w, b, M, a, N);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat_x86_sse)
		return dsp::simd::detail::x86_sse_filter_df2(w, b, M, a, N);
#endif // DSP_ARCH_FAMILY_X86
	else
		return dsp::filter_sample_df2(w, b, M, a, N);
}

void dsp::block_filter<float>::operator()()
{
	std::memmove(w_ + L_, w_, (P_ - 1) * sizeof(float));
	float* w = w_ + L_ - 1;
	float* x = x_;
	for (size_t n = 0; n != L_; ++n, --w, ++x) {
		*w = *x;
		*x = dsp::simd::filter_sample_df2(w, b_, M_pad_, a_, N_pad_);
	}
}
