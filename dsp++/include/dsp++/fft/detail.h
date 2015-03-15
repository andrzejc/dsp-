/// @file detail.h
/// @brief Implementation details of class fft
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_FFT_DETAIL_H_INCLUDED
#define DSP_FFT_DETAIL_H_INCLUDED

#include <dsp++/export.h>
#include <complex>
#include <cstddef>

namespace dsp { 

using std::complex;

namespace dft {

template<class Input, class Output> class fft;
template<class Real> class fft<complex<Real>, complex<Real> >;

/// @internal Implementation details. Do not use.
namespace detail {

/// @internal
template<class Real>
class fft_impl {
public:
	virtual ~fft_impl();

private:
	virtual void fft(complex<Real>* in_out, enum_class_ref(sign) sign) const = 0;
	friend class dsp::dft::fft<complex<Real>, complex<Real> >;
	static const fft_impl& get(size_t n);
};

/// @internal
template<>
class DSPXX_API fft_impl<float> {
public:
	virtual ~fft_impl();
private:
	virtual void fft(complex<float>* in_out, enum_class_ref(sign) sign) const = 0;
	friend class dsp::dft::fft<complex<float>, complex<float> >;
	static const fft_impl& get(size_t n);
};

/// @internal
template<>
class DSPXX_API fft_impl<double> {
public:
	virtual ~fft_impl();
private:
	virtual void fft(complex<double>* in_out, enum_class_ref(sign) sign) const = 0;
	friend class dsp::dft::fft<complex<double>, complex<double> >;
	static const fft_impl& get(size_t n);
};

}}}

#endif /* DSP_FFT_DETAIL_H_INCLUDED */
