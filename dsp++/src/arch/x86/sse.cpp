#include <dsp++/platform.h>

#ifdef DSP_ARCH_FAMILY_X86

#include <dsp++/simd.h>
#include "sse.h"
#include "sse_utils.h"

#include <xmmintrin.h>

void* dsp::simd::detail::x86_sse_alloc(int size, int align) {
	return _mm_malloc(size, align);
}

void dsp::simd::detail::x86_sse_free(void *p) {
	_mm_free(p);
}

//! @brief Piecewise vector multiplication using SSE instructions
void dsp::simd::detail::x86_sse_mulf(float* res, const float* x, const float* b, size_t N)
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
void dsp::simd::detail::x86_sse_mulcf(std::complex<float>* res_c, const std::complex<float>* a_c, const std::complex<float>* b_c, size_t len)
{
	float *res = reinterpret_cast<float*>(res_c);
	const float* a = reinterpret_cast<const float*>(a_c);
	const float* b = reinterpret_cast<const float*>(b_c);

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
std::complex<float> dsp::simd::detail::x86_sse_dotcf(const std::complex<float>* a_c, const std::complex<float>* b_c, size_t len)
{
	const float* a = reinterpret_cast<const float*>(a_c);
	const float* b = reinterpret_cast<const float*>(b_c);
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
float dsp::simd::detail::x86_sse_dotf(const float* x, const float* b, size_t N)
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

float dsp::simd::detail::x86_sse_filter_df2(float* w, const float* b, const size_t M, const float* a, const size_t N)
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

float dsp::simd::detail::x86_sse_filter_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step)
{
	__m128 cx, wx, xx, slack;
	xx = _mm_set_ss(x);				// xx[0] = x; xx[1-3] = 0; xx[0] will hold the result between the steps
	for (size_t i = 0; i < N; ++i, w += step, b += step, a += step, ++scale_only) {
		if (*scale_only) {
			cx = _mm_load_ss(b);		// load only 0th element from b
			xx = _mm_mul_ss(xx, cx);	// xx[0] *= cx[0]; 		don't need to write intermediate results back to w for scale-only section (we don't use it)
		}
		else {
			cx = _mm_load_ps(a);
			wx = _mm_load_ps(w);
			wx = _mm_move_ss(wx, xx);	// put result of previous iteration in wx[0]
			cx = _mm_mul_ps(cx, wx);
			SSE_HSUM(cx, cx, slack);	// cx[0] now has dot(a, w)
			wx = _mm_sub_ss(wx, cx);	// wx[0] -= dot(a, w);
			_mm_store_ps(w, wx);		// update the delay line for next sample, write wx to memory

			cx = _mm_load_ps(b);
			cx = _mm_mul_ps(cx, wx);	//
			SSE_HSUM(xx, cx, slack);	// xx[0] = dot(b, w)
		}
	}
	return _mm_cvtss_f32(xx);
}


#endif // DSP_ARCH_FAMILY_X86
