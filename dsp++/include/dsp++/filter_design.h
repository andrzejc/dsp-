/*!
 * @file dsp++/filter_design.h
 * @brief Filter design algorithms.
 */
#ifndef DSP_FILTER_DESIGN_H_INCLUDED
#define DSP_FILTER_DESIGN_H_INCLUDED

#include <dsp++/export.h>
#include <cstddef>
#include <complex>

namespace dsp {

/*!
 * @brief Specifies type of the filter to design.
 */
enum filter_type
{
	filter_type_default,      //!< Ordinary (Type I or II) band-pass filter.
	filter_type_hilbert,      //!< Hilbert transformer (Type III).
	filter_type_differentiator//!< Differentiator (Type IV).
};

/*!
 * @brief FIR filter design with Parks-McClellan algorithm.
 * @return false if max iteration count was reached (results may be imprecise).
 * @throw std::bad_alloc if unable to allocate memory for internal arrays.
 * @throw std::domain_error if any frequency in freqs is outside [0, 0.5] range or any weight in weights is not greater than 0.
 * @throw std::invalid_argument if freqs is not a monotonically increasing sequence.
 * @note this function is named after MATLAB counterpart.
 * TODO provide additional documentation for this non-trivial task.
 */
DSPXX_API bool firpm(
		size_t order, 				//!< [in] filter order, number of coefficients will be order + 1.
		double h[], 				//!< [out] designed filter impulse response [order + 1].
		size_t band_count, 			//!< [in] number of bands in the filter specification.
		double freqs[], 			//!< [in] band edges [band_count * 2], all freqs must fall into [0, 0.5] range.
		const double amps[], 		//!< [in] amplitude characteristic at each band edge [band_count * 2].
		const double weights[], 	//!< [in] error weights for each band [band_count].
		filter_type type = filter_type_default, 	//!< [in] type of filter to design.
		size_t grid_density = 16, 	//!< [in] initial grid density.
		size_t max_iterations = 32 	//!< [in] max iteration count.
		);

/*!
 * @brief FIR filter design with frequency sampling method with result in the spectrum domain.
 * @throw std::bad_alloc if unable to allocate memory for internal arrays.
 * @throw std::domain_error if any frequency in freqs is outside [0, 0.5] range.
 * @throw std::invalid_argument if freqs is not a monotonically increasing sequence or (point_count < 2) or (freqs[0] != 0) or (freqs[point_count - 1] != 0.5)
 * @return number of frequency response samples returned in H (which typically is @p order + 1, but may be @p order in case there's 0 at DC and @p order is odd).
 * @note Unlike its MATLAB counterpart, this function will not do anything to widen transition region in case the modeled amplitude response
 *		has steps. This is the caller's responsibility to design the transition region.
 * @see http://www.mathworks.com/help/signal/ref/fir2.html
 */
DSPXX_API size_t fir_freq_samp(
		size_t order,				//!< [in] filter order, number of coefficients will be order + 1.
		std::complex<double> H[], 	//!< [out] filter response designed in the spectrum domain [order + 1] or [nfft * 2] if nfft set.
		size_t point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[]			//!< [in] amplitude characteristic at each frequency point
		);
///		size_t nfft = 0				//!< [in] if set, specifies number of points of the frequency characteristic being sampled/interpolated; in such case H should be set to twice of that.


/*!
 * @brief FIR filter design with frequency sampling method.
 * @throw std::bad_alloc if unable to allocate memory for internal arrays.
 * @throw std::domain_error if any frequency in freqs is outside [0, 0.5] range.
 * @throw std::invalid_argument if freqs is not a monotonically increasing sequence or (point_count < 2) or (freqs[0] != 0) or (freqs[point_count - 1] != 0.5)
 * @return impulse response length (which typically is @p order + 1, but may be @p order in case there's 0 at DC and @p order is odd).
 * @note Unlike its MATLAB counterpart, this function will not do anything to widen transition region in case the modeled amplitude response
 *		has steps. This is the caller's responsibility to design the transition region.
 * @see http://www.mathworks.com/help/signal/ref/fir2.html
 */
DSPXX_API size_t fir_freq_samp(
		size_t order,				//!< [in] filter order, number of coefficients will be order + 1.
		double h[], 				//!< [out] designed filter impulse response [order + 1].
		size_t point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		const double wnd[]			//!< [in] window function to apply to the impulse response, needs to be of appropriate length. If explicitly set to NULL, no windowing will be used.
		);

/*!
 * @brief FIR filter design with frequency sampling method and Hamming window applied to impulse response.
 * @throw std::bad_alloc if unable to allocate memory for internal arrays.
 * @throw std::domain_error if any frequency in freqs is outside [0, 0.5] range.
 * @throw std::invalid_argument if freqs is not a monotonically increasing sequence or (point_count < 2) or (freqs[0] != 0) or (freqs[point_count - 1] != 0.5)
 * @return impulse response length (which typically is @p order + 1, but may be @p order in case there's 0 at DC and @p order is odd).
 * @note Unlike its MATLAB counterpart, this function will not do anything to widen transition region in case the modeled amplitude response
 *		has steps. This is the caller's responsibility to design the transition region.
 * @see http://www.mathworks.com/help/signal/ref/fir2.html
 */
DSPXX_API size_t fir_freq_samp(
		size_t order,				//!< [in] filter order, number of coefficients will be order + 1.
		double h[], 				//!< [out] designed filter impulse response [order + 1].
		size_t point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[]			//!< [in] amplitude characteristic at each frequency point
		);

/*!
 * @brief Specifies type of biquad section to design with biquad_design().
 */
enum biquad_type
{
	biquad_type_lowpass,      //!< Lowpass biquad filter.
	biquad_type_highpass,     //!< Highpass biquad filter.
	biquad_type_bandpass,     //!< Bandpass biquad filter.
	biquad_type_notch,        //!< Notch biquad filter.
	biquad_type_allpass,      //!< Allpass biquad filter.
	biquad_type_peaking_eq,   //!< Peaking equalizer.
	biquad_type_low_shelf_eq, //!< Low shelf equalizer.
	biquad_type_high_shelf_eq,//!< High shelf equalizer.
};

/*!
 * @brief Biquad section design.
 * This code is based on "Cookbook formulae for audio EQ biquad filter coefficients" by Robert Bristow-Johnson.
 * @see http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 * @note The params q, bw and s are interchangable (s may be used for eq filters only, though). Nevertheless one of
 * them must be set. The param gain_db is used only for eq filters too, it its required then.
 * @throw std::domain_error if norm_freq is outside [0, 0.5] range
 * @throw std::invalid_argument if gain_db or one of q, bw, s is not provided for equalizer filter.
 */
DSPXX_API void biquad_design(
		double b[],				//!< [out] difference equation numerator (FIR) polynomial coefficients [3].
		double a[], 			//!< [out] difference equation denominator (IIR) polynomial coefficients [3].
		biquad_type type, 		//!< [in] type of biquad section to design.
		double norm_freq,		//!< [in] normalized characteristic frequency of designed filter (@f$f_0/F_s@f$; center frequency, corner frequency, shelf midpoint frequency).
		const double* gain_db, 	//!< [in] gain in dB, used only for peaking and shelving eq filters.
		const double* q,		//!< [in] filter quality.
		const double* bw,		//!< [in] filter bandwidth in octaves, between -3dB freqs in case of bandpass and notch or between midpoint (gain_db/2) freqs in case of shelfs.
		const double* s			//!< [in] shelf slope, if set to 1 - as steep as it can be, proportional to slope in dB/octave.
);

//! @brief Specifies type of filter and design flags to filter design design with iir_design()
enum iir_type 
{	
	iir_butterworth		=	0,	//!< Design Butterworth LP, HP, BP or BS filter
	iir_bessel			=	1,	//!< Design Bessel LP, HP, BP or BS filter
	iir_chebyshev		=	2,	//!< Design Chebyshev LP, HP, BP or BS filter

	iir_lowpass			=  16,	//!< Lowpass characteristic
	iir_highpass		=  32,	//!< Highpass characteristic
	iir_bandpass		=  64,	//!< Bandpass characteristic
	iir_bandstop		= 128,	//!< Bandstop characteristic
	iir_allpass			= 256,	//!< Allpass characteristic, only in conjunction with {@link iir_resonator_design()}

	iir_prewrap			= 512,	//!< 
	iir_matched_z		=1024,	//!< Use matched Z-transform instead of bilinear transform
};

//! @brief Design IIR resonator filter in the z-plane
// XXX check number of coefficients
DSPXX_API void iir_resonator_design(
	double b[],					//!< [out] difference equation numerator (FIR) polynomial coefficients
	double a[],					//!< [out] difference equation denominator (IIR) polynomial coefficients
	iir_type characteristic,	//!< [in] filter characteristic ({@link iir_bandstop} (notch), {@link iir_bandpass} or {@link iir_allpass})
	double fc,					//!< [in] normalized centre frequency (0, 0.5) range
	double q					//!< [in] quality factor
);

//! @brief Design IIR Butterworth, Bessel or Chebyshev filter in the z-plane according to specification.
DSPXX_API void iir_filter_design(
	size_t order,				//!< [in] filter order
	double b[],					//!< [out] difference equation numerator (FIR) polynomial coefficients (order + 1) or (2 * order + 1) in case of BP, BS
	double a[],					//!< [out] difference equation denominator (IIR) polynomial coefficients (order + 1) or (2 * order + 1) in case of BP, BS
	unsigned type,				//!< [in] filter type, characteristic and flags (combination of reasonable {@link iir_type} values)
	const double* fc,				//!< [in] normalized corner frequency/ies (0, 0.5) range
	const double* cheb_rip = NULL,	//!< [in] Chebyshev ripple in dB
	const double* zero_freq = NULL,	//!< [in] put additional zero at specified normalized frequency
	unsigned pole_mask = 0		//!< [in] Use only specified poles
);

//! @brief Design IIR Butterworth, Bessel or Chebyshev filter in the z-plane according to specification.
//! @return gain
DSPXX_API double iir_filter_design(
	size_t order,				//!< [in] filter order
	std::complex<double> z[],	//!< [out] z-plane zeros (order) or (2 * order) in case of BP, BS
	std::complex<double> p[],	//!< [out] z-plane poles (order) or (2 * order) in case of BP, BS
	unsigned type,				//!< [in] filter type, characteristic and flags (combination of reasonable {@link iir_type} values)
	const double* fc,				//!< [in] normalized corner frequency/ies (0, 0.5) range
	const double* cheb_rip = NULL,	//!< [in] Chebyshev ripple in dB
	const double* zero_freq = NULL,	//!< [in] put additional zero at specified normalized frequency
	unsigned pole_mask = 0		//!< [in] Use only specified poles
);

}

#endif /* DSP_FILTER_DESIGN_H_INCLUDED */
