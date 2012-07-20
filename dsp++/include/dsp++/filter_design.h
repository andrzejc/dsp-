/*!
 * @file dsp++/filter_design.h
 * @brief Filter design algorithms.
 */
#ifndef DSP_FILTER_DESIGN_H_INCLUDED
#define DSP_FILTER_DESIGN_H_INCLUDED

#include <dsp++/export.h>
#include <cstddef>

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
	 * @note this function is named after MATLAB counterpart.
	 */
	DSPXX_API bool firpm(
			size_t order, 				//!< [in] filter order, number of coefficients will be order + 1.
			double h[], 				//!< [out] designed filter impulse response [order + 1].
			size_t band_count, 			//!< [in] number of bands in the filter specification.
			double freqs[], 			//!< [in] band edges [band_count * 2].
			const double amps[], 		//!< [in] amplitude characteristic at each band edge [band_count * 2].
			const double weights[], 	//!< [in] error weights for each band [band_count].
			filter_type type = filter_type_default, 	//!< [in] type of filter to design.
			size_t grid_density = 16, 	//!< [in] initial grid density.
			size_t max_iterations = 32 	//!< [in] max iteration count.
			);

	enum biquad_type
	{
		biquad_type_lowpass,
		biquad_type_highpass,
		biquad_type_bandpass,
		biquad_type_notch,
		biquad_type_allpass,
		biquad_type_peaking_eq,
		biquad_type_low_shelf_eq,
		biquad_type_high_shelf_eq,
	};

	DSPXX_API void biquad_design(
			double b[],				//!< [out] difference equation numerator (FIR) polynomial coefficients [3].
			double a[], 			//!< [out] difference equation denominator (IIR) polynomial coefficients [3].
			biquad_type type, 		//!< [in] type of biquad section to design.
			double norm_freq,		//!< [in] normalized characteristic frequency of designed filter (@f$f_0/F_s@f$; center frequency, corner frequency, shelf midpoint frequency).
			const double* gain_db, 	//!< [in] gain in dB, used only for peaking and shelving eq filters.
			const double* q,		//!< [in] filter quality.
			const double* bw,		//!< [in] filter bandwidth in octaves.
			const double* s			//!< [in] shelf slope, if set to 1 - as steep as it can be, proportional to slope in dB/octave.
	);

}

#endif /* DSP_FILTER_DESIGN_H_INCLUDED */
