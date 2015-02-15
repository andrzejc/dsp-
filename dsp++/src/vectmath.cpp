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
#include "arch/x86/sse.h"
#endif // DSP_ARCH_FAMILY_X86

#define noop() ((void)0)

namespace dsp { namespace simd { namespace detail { } } }
using namespace dsp::simd::detail;

float dsp::simd::dot(const float* v0, const float* v1, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	// TODO uncomment SSE4.1 block in dsp::simd::dot() when x86_sse41_dotf() is optimized 
	//else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse41)
	//	return x86_sse41_dotf(v0, v1, len);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse3)
		return x86_sse3_dotf(v0, v1, len);
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		return x86_sse_dotf(v0, v1, len);
#endif // DSP_ARCH_FAMILY_X86

	return dsp::dot(v0, v1, len);
}

std::complex<float> dsp::simd::dot(const std::complex<float>* a, const std::complex<float>* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		return x86_sse_dotcf(a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	return dsp::dot(a, b, len);

}

void dsp::simd::mul(float* res, const float* a, const float* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_mulf(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::mul(res, a, b, len);
}

void dsp::simd::mul(float* res, const float* a, float s, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_mulf(res, a, s, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::mul(res, a, s, len);
}

void dsp::simd::div(float* res, const float* a, const float* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_divf(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::div(res, a, b, len);
}

void dsp::simd::div(float* res, const float* a, float s, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_divf(res, a, s, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::div(res, a, s, len);
}

void dsp::simd::add(float* res, const float* a, const float* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_addf(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::add(res, a, b, len);
}

void dsp::simd::add(float* res, const float* a, float s, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_addf(res, a, s, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::add(res, a, s, len);
}

void dsp::simd::sub(float* res, const float* a, const float* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_subf(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::sub(res, a, b, len);
}

void dsp::simd::sub(float* res, const float* a, float s, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_subf(res, a, s, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::sub(res, a, s, len);
}

void dsp::simd::mul(std::complex<float>* res, const std::complex<float>* a, const std::complex<float>* b, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_mulcf(res, a, b, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::mul(res, a, b, len);
}

void dsp::simd::sqrt(float* res, const float* a, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_sqrtf(res, a, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::sqrt(res, a, len);
}

void dsp::simd::recip(float* res, const float* a, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_rcpf(res, a, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::recip(res, a, len);
}

void dsp::simd::rsqrt(float* res, const float* a, size_t len)
{
	if (false) noop();
#ifdef DSP_ARCH_FAMILY_X86
	else if (DSP_SIMD_FEATURES & dsp::simd::feat::x86_sse)
		x86_sse_rsqrtf(res, a, len);
#endif // DSP_ARCH_FAMILY_X86

	dsp::rsqrt(res, a, len);
}
