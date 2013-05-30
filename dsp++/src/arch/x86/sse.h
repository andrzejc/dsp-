#ifndef DSP_INTERNAL_X86_SSE_H_INCLUDED
#define DSP_INTERNAL_X86_SSE_H_INCLUDED
#pragma once

#include <complex>
#include <cstddef>

namespace dsp { namespace simd { namespace detail {

void* x86_sse_alloc(int size, int align);
void x86_sse_free(void *p);

void x86_sse_mulf(float* res, const float* x, const float* b, size_t N);
void x86_sse_mulf(float* res, const float* a, float s, size_t len);
void x86_sse_mulcf(std::complex<float>* res, const std::complex<float>* a, const std::complex<float>* b, size_t len);

std::complex<float> x86_sse_dotcf(const std::complex<float>* a, const std::complex<float>* b, size_t len);
float x86_sse_dotf(const float* x, const float* b, size_t N);

float x86_sse_filter_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step);

float x86_sse3_dotf(const float* x, const float* b, size_t N);
float x86_sse41_dotf(const float* x, const float* b, size_t N);

void x86_sse_sqrtf(float* res, const float* a, size_t len);

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
float x86_sse_filter_df2(float* w, const float* b, const size_t M, const float* a, const size_t N);
float x86_sse41_filter_df2(float* w, const float* b, const size_t M, const float* a, const size_t N);

} } }

#endif /* DSP_INTERNAL_X86_SSE_H_INCLUDED */
