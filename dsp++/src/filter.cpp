/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/simd.h>
#include <cstring>

#include "arch/x86/sse.h"

#define noop() ((void)0)

using namespace dsp;

float dsp::simd::filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
{
	if (false) noop();
#ifdef DSPXX_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse41)
		return dsp::simd::detail::x86_sse41_filter_df2(w, b, M, a, N);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		return dsp::simd::detail::x86_sse_filter_df2(w, b, M, a, N);
#endif // DSPXX_ARCH_FAMILY_X86

	return dsp::filter_sample_df2(w, b, M, a, N);
}

float dsp::simd::filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N, int feat_flags)
{
	if (false) noop();
#ifdef DSPXX_ARCH_FAMILY_X86
	else if (feat_flags & dsp::simd::feat::x86_sse41)
		return dsp::simd::detail::x86_sse41_filter_df2(w, b, M, a, N);
	else if (feat_flags & dsp::simd::feat::x86_sse)
		return dsp::simd::detail::x86_sse_filter_df2(w, b, M, a, N);
#endif // DSPXX_ARCH_FAMILY_X86

	return dsp::filter_sample_df2(w, b, M, a, N);
}

namespace dsp { namespace detail {


} }

float dsp::simd::filter_sample_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step)
{
	if (false) noop();
#ifdef DSPXX_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		return dsp::simd::detail::x86_sse_filter_sos_df2(x, N, scale_only, w, b, a, step);
#endif // DSPXX_ARCH_FAMILY_X86

	return dsp::filter_sample_sos_df2(x, N, scale_only, w, b, a, step);
}

float dsp::simd::filter_sample_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step, int feat_flags)
{
	if (false) noop();
#ifdef DSPXX_ARCH_FAMILY_X86
	else if (feat_flags & dsp::simd::feat::x86_sse)
		return dsp::simd::detail::x86_sse_filter_sos_df2(x, N, scale_only, w, b, a, step);
#endif // DSPXX_ARCH_FAMILY_X86

	return dsp::filter_sample_sos_df2(x, N, scale_only, w, b, a, step);
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

float dsp::filter_sos<float>::operator()(float x)
{
	std::memmove(w_ + 1, w_, (N_ * step_ - 1) * sizeof(float));
	return dsp::simd::filter_sample_sos_df2(x, N_, scale_only_.get(), w_, b_, a_, step_);
}
