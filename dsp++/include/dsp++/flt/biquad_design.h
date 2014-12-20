#ifndef DSP_FLT_BIQUAD_DESIGN_H_INCLUDED
#define DSP_FLT_BIQUAD_DESIGN_H_INCLUDED
#pragma once
#include <dsp++/export.h>
#include <cstddef>

namespace dsp {

//! @brief Tools for designing biquad (second-order) filters
namespace biquad {


//! @brief Specifies type of biquad section to design with biquad_design().
namespace type { enum spec
{
	lowpass,      //!< Lowpass biquad filter.
	highpass,     //!< Highpass biquad filter.
	bandpass,     //!< Bandpass biquad filter.
	notch,        //!< Notch biquad filter.
	allpass,      //!< Allpass biquad filter.
	peaking_eq,   //!< Peaking equalizer.
	low_shelf_eq, //!< Low shelf equalizer.
	high_shelf_eq,//!< High shelf equalizer.
};}

/*!
 * @brief Biquad section design.
 * This code is based on "Cookbook formulae for audio EQ biquad filter coefficients" by Robert Bristow-Johnson.
 * @see http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
 * @note The params q, bw and s are interchangable (s may be used for eq filters only, though). Nevertheless one of
 * them must be set. The param gain_db is used only for eq filters too, it its required then.
 * @throw std::domain_error if norm_freq is outside [0, 0.5] range
 * @throw std::invalid_argument if gain_db or one of q, bw, s is not provided for equalizer filter.
 */
DSPXX_API void design(
		double b[],				//!< [out] difference equation numerator (FIR) polynomial coefficients [3].
		double a[], 			//!< [out] difference equation denominator (IIR) polynomial coefficients [3].
		type::spec type, 		//!< [in] type of biquad section to design.
		double norm_freq,		//!< [in] normalized characteristic frequency of designed filter (@f$f_0/F_s@f$; center frequency, corner frequency, shelf midpoint frequency).
		const double* gain_db, 	//!< [in] gain in dB, used only for peaking and shelving eq filters.
		const double* q,		//!< [in] filter quality.
		const double* bw,		//!< [in] filter bandwidth in octaves, between -3dB freqs in case of bandpass and notch or between midpoint (gain_db/2) freqs in case of shelfs.
		const double* s			//!< [in] shelf slope, if set to 1 - as steep as it can be, proportional to slope in dB/octave.
);

}}

#endif // DSP_FLT_BIQUAD_DESIGN_H_INCLUDED