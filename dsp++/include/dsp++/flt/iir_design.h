#ifndef DSP_FLT_IIR_DESIGN_H_INCLUDED
#define DSP_FLT_IIR_DESIGN_H_INCLUDED
#pragma once
#include <dsp++/export.h>
#include <cstddef>
#include <complex>

namespace dsp {

//! @brief IIR filter design utilities
namespace iir {

//! @brief IIR filter type constants for use with {@link design()}.
namespace type { enum spec 
{	
	butterworth		=	0,	//!< Design Butterworth LP, HP, BP or BS filter
	bessel			=	1,	//!< Design Bessel LP, HP, BP or BS filter
	chebyshev		=	2,	//!< Design Chebyshev LP, HP, BP or BS filter
};}

//! @brief Filter response characteristics constants for use with {@link design()}.
namespace resp { enum spec 
{
	lowpass			=  16,	//!< Lowpass characteristic
	highpass		=  32,	//!< Highpass characteristic
	bandpass		=  64,	//!< Bandpass characteristic
	bandstop		= 128,	//!< Bandstop characteristic
	allpass			= 256,	//!< Allpass characteristic, only in conjunction with {@link resonator_design()}
};}

//! @brief IIR filter design flags for use with {@link design()}.
namespace flag { enum spec
{
	prewrap			= 512,	//!< 
	matched_z		=1024,	//!< Use matched Z-transform instead of bilinear transform
};}

//! @brief Design IIR resonator filter in the z-plane
// XXX check number of coefficients
DSPXX_API void resonator_design(
	unsigned order,
	resp::spec characteristic,		//!< [in] filter characteristic ({@link iir::resp::bandstop} (notch), {@link iir::resp::bandpass} or {@link iir::resp::allpass})
	double fc,						//!< [in] normalized centre frequency (0, 0.5) range
	double q,						//!< [in] quality factor
	double b[],						//!< [out] difference equation numerator (FIR) polynomial coefficients
	double a[]						//!< [out] difference equation denominator (IIR) polynomial coefficients
);

//! @brief Design IIR Butterworth, Bessel or Chebyshev filter in the z-plane according to specification.
//! @throw std::domain_error if frequency in @p fc or @p zero_freq falls outside [0, 0.5]
//! @throw std::logic_error if @p type flags are incomplete or freqs in @p fc are non-increasing with bandpass/bandstop filters
//! @return required length of @p b and @p a vectors.
DSPXX_API unsigned design(
	unsigned order,				//!< [in] filter order
	unsigned type,				//!< [in] filter type, characteristic and flags (combination of reasonable {@link iir::type}, {@link iir::resp} &amp; {@link iir::flag} values)
	const double* fc,			//!< [in] normalized corner frequency/ies (0, 0.5) range
	double b[],					//!< [out] difference equation numerator (FIR) polynomial coefficients (@p order + 1) or (2 * @p order + 1) in case of BP, BS; if @p NULL only required length is returned; length increased by 2 if @p zero_freq is not @p NULL.
	double a[],					//!< [out] difference equation denominator (IIR) polynomial coefficients (@p order + 1) or (2 * @p order + 1) in case of BP, BS; if @p NULL only required length is returned; length increased by 2 if @p zero_freq is not @p NULL.
	const double* cheb_rip = NULL,	//!< [in] Chebyshev ripple in dB, defaults to 3dB if not set
	const double* zero_freq = NULL,	//!< [in] put additional zero at specified normalized frequency
	unsigned pole_mask = 0		//!< [in] Use only specified poles
);

//! @brief Design IIR Butterworth, Bessel or Chebyshev filter in the z-plane according to specification.
//! @throw std::domain_error if frequency in @p fc or @p zero_freq falls outside [0, 0.5]
//! @throw std::logic_error if @p type flags are incomplete or freqs in @p fc are non-increasing with bandpass/bandstop filters
//! @return gain
DSPXX_API double design(
	unsigned order,				//!< [in] filter order
	unsigned type,				//!< [in] filter type, characteristic and flags (combination of reasonable {@link iir::type}, {@link iir::resp} &amp; {@link iir::flag} values)
	const double* fc,			//!< [in] normalized corner frequency/ies (0, 0.5) range
	std::complex<double> z[],	//!< [out] z-plane zeros (@p order) or (2 * @p order) in case of BP, BS; length increased by 2 if @p zero_freq is not @p NULL.
	std::complex<double> p[],	//!< [out] z-plane poles (@p order) or (2 * @p order) in case of BP, BS; length increased by 2 if @p zero_freq is not @p NULL.
	const double* cheb_rip = NULL,	//!< [in] Chebyshev ripple in dB, defaults to 3dB if not set
	const double* zero_freq = NULL,	//!< [in] put additional zero at specified normalized frequency
	unsigned pole_mask = 0		//!< [in] Use only specified poles
);

//! @return number of second-order-sections written, or going to be written to @p num, @p den (ceil(order / 2)); needs to be multiplied by @p sos_length to get @p num/@p den length
//! @throw std::domain_error if frequency in @p fc or @p zero_freq falls outside [0, 0.5]
//! @throw std::logic_error if @p type flags are incomplete or freqs in @p fc are non-increasing with bandpass/bandstop filters
//! @see dsp::zp2sos()
DSPXX_API unsigned sos_design(
	unsigned order,				//!< [in] filter order
	unsigned type,				//!< [in] filter type, characteristic and flags (combination of reasonable {@link iir::type}, {@link iir::resp} &amp; {@link iir::flag} values)
	const double* fc,			//!< [in] normalized corner frequency/ies (0, 0.5) range
	double num[],					//!< [out] 
	double den[],					//!< [out] 
	const double* cheb_rip = NULL,	//!< [in] Chebyshev ripple in dB, defaults to 3dB if not set
	const double* zero_freq = NULL,	//!< [in] put additional zero at specified normalized frequency
	unsigned pole_mask = 0		//!< [in] Use only specified poles
);


}} // namespace dsp::iir

#endif // DSP_FLT_IIR_DESIGN_H_INCLUDED