/*!
 * @file detail.h
 * @brief Implementation details of class fft
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_FFT_DETAIL_H_INCLUDED
#define DSP_FFT_DETAIL_H_INCLUDED

#include <dsp++/export.h>
#include <complex>

namespace dsp {

template<class Input, class Output> class fft;
template<class Real> class fft<std::complex<Real>, std::complex<Real> >;

//! @internal Implementation details. Do not use.
namespace detail {

//! @internal
template<class Real>
class fft_impl {
public:
	virtual ~fft_impl();

private:
	virtual void fft(std::complex<Real>* in_out, int sign) const = 0;
	friend class dsp::fft<std::complex<Real>, std::complex<Real> >;
	static const fft_impl& get(size_t n);
};

//! @internal
template<>
class DSPXX_API fft_impl<float> {
public:
	virtual ~fft_impl();
private:
	virtual void fft(std::complex<float>* in_out, int sign) const = 0;
	friend class dsp::fft<std::complex<float>, std::complex<float> >;
	static const fft_impl& get(size_t n);
};

//! @internal
template<>
class DSPXX_API fft_impl<double> {
public:
	virtual ~fft_impl();
private:
	virtual void fft(std::complex<double>* in_out, int sign) const = 0;
	friend class dsp::fft<std::complex<double>, std::complex<double> >;
	static const fft_impl& get(size_t n);
};

}}

#endif /* DSP_FFT_DETAIL_H_INCLUDED */
