/*!
 * @file fft.cpp
 * @brief Low-overhead FFT algorithm based on source code published
 * by Vladimir Mirnyi in Dr Dobbs C++ Journal on May 25, 2007
 * @see http://drdobbs.com/cpp/199702312
 * @author (of dsp++ wrapper) Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 * @author (of original code) Vladimir Mirnyi (see copyright notice below)
 * @copyright Copyright &copy; 2006 by Volodymyr Myrnyy (Vladimir Mirnyi)
 * Permission to use, copy, modify, distribute and sell this software for any
 * purpose is hereby granted without fee, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and this
 * permission notice appear in supporting documentation.
 */

#include <dsp++/fft.h>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <dsp++/pow2.h>
#include <dsp++/const.h>

namespace {

	////// template class sin_cos_series
// common series to compile-time calculation
// of sine and cosine functions
template<unsigned M, unsigned N, unsigned B, unsigned A>
struct sin_cos_series
{
	static double value()
	{
		return 1 - (A * DSP_M_PI / B) * (A * DSP_M_PI / B) / M / (M + 1) * sin_cos_series<M + 2, N, B, A>::value();
	}
};

template<unsigned N, unsigned B, unsigned A>
struct sin_cos_series<N, N, B, A>
{
	static double value()
	{
		return 1.;
	}
};

////// template class sin
// compile-time calculation of sin(A*M_PI/B) function
template<unsigned B, unsigned A, typename T = double>
struct sin;

template<unsigned B, unsigned A>
struct sin<B, A, float>
{
	static float value()
	{
		return static_cast<float>((A * DSP_M_PI / B) * sin_cos_series < 2, 24, B, A > ::value());
	}
};
template<unsigned B, unsigned A>
struct sin<B, A, double>
{
	static double value()
	{
		return (A * DSP_M_PI / B) * sin_cos_series < 2, 34, B, A > ::value();
	}
};

////// template class cos
// compile-time calculation of cos(A*M_PI/B) function
template<unsigned B, unsigned A, typename T = double>
struct cos;

template<unsigned B, unsigned A>
struct cos<B, A, float>
{
	static float value()
	{
		return sin_cos_series < 1, 23, B, A > ::value();
	}
};
template<unsigned B, unsigned A>
struct cos<B, A, double>
{
	static double value()
	{
		return sin_cos_series < 1, 33, B, A > ::value();
	}
};

////// template class danielson_lanczos
// Danielson-Lanczos section of the FFT
template<unsigned N, typename T = double>
class danielson_lanczos
{
	danielson_lanczos<N / 2, T> next_;
public:
	void apply(T* data) const
	{
		next_.apply(data);
		next_.apply(data + N);

		T wtemp, tempr, tempi, wr, wi, wpr, wpi;
		//    Change dynamic calculation to the static one
		//      wtemp = sin(M_PI/N);
		wtemp = -sin < N, 1, T > ::value();
		wpr = static_cast<T>(-2.0) * wtemp * wtemp;
		//      wpi = -sin(2*M_PI/N);
		wpi = -sin < N, 2, T > ::value();
		wr = 1.0;
		wi = 0.0;
		for (unsigned i = 0; i < N; i += 2)
		{
			tempr = data[i + N] * wr - data[i + N + 1] * wi;
			tempi = data[i + N] * wi + data[i + N + 1] * wr;
			data[i + N] = data[i] - tempr;
			data[i + N + 1] = data[i + 1] - tempi;
			data[i] += tempr;
			data[i + 1] += tempi;

			wtemp = wr;
			wr += wr * wpr - wi * wpi;
			wi += wi * wpr + wtemp * wpi;
		}
	}
};

template<typename T>
class danielson_lanczos<4, T>
{
public:
	void apply(T* data) const
	{
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0] - tr;
		data[3] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[5] - ti;
		data[7] = tr - data[4];
		data[4] += tr;
		data[5] += ti;

		tr = data[4];
		ti = data[5];
		data[4] = data[0] - tr;
		data[5] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
		tr = data[6];
		ti = data[7];
		data[6] = data[2] - tr;
		data[7] = data[3] - ti;
		data[2] += tr;
		data[3] += ti;
	}
};

template<typename T>
class danielson_lanczos<2, T>
{
public:
	void apply(T* data) const
	{
		T tr = data[2];
		T ti = data[3];
		data[2] = data[0] - tr;
		data[3] = data[1] - ti;
		data[0] += tr;
		data[1] += ti;
	}
};

// generic fast Fourier transform main class
template<unsigned P, typename T = double>
class fft_impl: public dsp::detail::fft_impl<T>
{
	enum
	{
		N = 1 << P
	};
	danielson_lanczos<N, T> recursion_;
	void scramble(T* data) const
	{
		int i, m, j = 1;
		for (i = 1; i < 2 * N; i += 2)
		{
			if (j > i)
			{
				std::swap(data[j - 1], data[i - 1]);
				std::swap(data[j], data[i]);
			}
			m = N;
			while (m >= 2 && j > m)
			{
				j -= m;
				m >>= 1;
			}
			j += m;
		}
	}

	void swap_real_imag(std::complex<T>* data) const
	{
		std::complex<T>* end = data + N;
		for (; data != end; ++data)
			*data = std::complex<T>(data->imag(), data->real());
	}

public:
	void fft(std::complex<T>* in_out, int sign) const
	{
		if (dsp::dft_sign_backward == sign)
			swap_real_imag(in_out);
		scramble(reinterpret_cast<T*>(in_out));
		recursion_.apply(reinterpret_cast<T*>(in_out));
		if (dsp::dft_sign_backward == sign)
			swap_real_imag(in_out);
	}
};

#define FFT_IMPL_NAME(p, type) type ## p
#define FFT_IMPL_INST_PREFIX(p, type) const fft_impl<p, type>
#define FFT_IMPL_INST_POSTFIX(p, type) ;

#define FFT_IMPL_INST(p, type) FFT_IMPL_INST_PREFIX(p, type) FFT_IMPL_NAME(p, type) FFT_IMPL_INST_POSTFIX(p, type)

#define FFT_IMPL_TYPE(type) \
	FFT_IMPL_INST( 1, type) FFT_IMPL_INST( 2, type) FFT_IMPL_INST( 3, type) FFT_IMPL_INST( 4, type) \
	FFT_IMPL_INST( 5, type) FFT_IMPL_INST( 6, type) FFT_IMPL_INST( 7, type) FFT_IMPL_INST( 8, type) \
	FFT_IMPL_INST( 9, type) FFT_IMPL_INST(10, type) FFT_IMPL_INST(11, type) FFT_IMPL_INST(12, type) \
	FFT_IMPL_INST(13, type) FFT_IMPL_INST(14, type) FFT_IMPL_INST(15, type) FFT_IMPL_INST(16, type) \
	FFT_IMPL_INST(17, type) FFT_IMPL_INST(18, type) FFT_IMPL_INST(19, type) FFT_IMPL_INST(20, type) \
	FFT_IMPL_INST(21, type) FFT_IMPL_INST(22, type) FFT_IMPL_INST(23, type) FFT_IMPL_INST(24, type) \
	FFT_IMPL_INST(25, type) FFT_IMPL_INST(26, type) 

	FFT_IMPL_TYPE(float);
	FFT_IMPL_TYPE(double);

#undef FFT_IMPL_INST_PREFIX
#undef FFT_IMPL_INST_POSTFIX

#define FFT_IMPL_INST_PREFIX(p, type) &
#define FFT_IMPL_INST_POSTFIX(p, type) ,

#define FFT_ARRAY_NAME(type) fft_impl_ ## type

#define FFT_IMPL_ARRAY(type) \
	const dsp::detail::fft_impl<type>* FFT_ARRAY_NAME(type)[] = { FFT_IMPL_TYPE(type) }

	FFT_IMPL_ARRAY(float);
	FFT_IMPL_ARRAY(double);

static size_t fft_impl_index(size_t size)
{
	if (!dsp::ispow2(size))
		throw std::domain_error("dsp::fft only allows transform size of integer powers of 2");
	if (size < 2 || size > 67108864)
		throw std::out_of_range("transform size outside [2^1, 2^26]");
	for (size_t u = 2, i = 0; ; ++i, u<<=1)
		if (u == size)
			return i;
	// we never get here
	return 0;
}

}

#define FFT_IMPL_DEFINE(type) \
	dsp::detail::fft_impl<type>::~fft_impl() {} \
	const dsp::detail::fft_impl<type>& dsp::detail::fft_impl<type>::get(size_t n) \
	{return *FFT_ARRAY_NAME(type)[fft_impl_index(n)];}

FFT_IMPL_DEFINE(float);
FFT_IMPL_DEFINE(double);
