#ifndef DSP_FLT_FIR_DESIGN_H_INCLUDED
#define DSP_FLT_FIR_DESIGN_H_INCLUDED
#pragma once
#include <dsp++/export.h>
#include <complex>
#include <cstddef>

namespace dsp {

//! @brief FIR filter design utilities
namespace fir {

//! @brief Parks-McClellan filter design algorithm
namespace pm {

//! @brief Specifies type of the filter to design.
namespace type { enum spec 
{
	type_I_II,      //!< Ordinary (Type I or II) band-pass filter.
	hilbert,		//!< Hilbert transformer (Type III).
	differentiator	//!< Differentiator (Type IV).
};}

static const unsigned grid_density_default = 16;
static const unsigned max_iterations_default = 32;

/*!
 * @brief FIR filter design with Parks-McClellan algorithm.
 * @return @p false if max iteration count was reached (results may be imprecise).
 * @throw std::bad_alloc if unable to allocate memory for internal arrays.
 * @throw std::domain_error if any frequency in freqs is outside [0, 0.5] range or any weight in weights is not greater than 0.
 * @throw std::invalid_argument if freqs is not a monotonically increasing sequence.
 * TODO provide additional documentation for this non-trivial task.
 */
DSPXX_API bool design(
		unsigned order, 				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned band_count, 			//!< [in] number of bands in the filter specification.
		double freqs[], 				//!< [in] band edges [band_count * 2], all freqs must fall into [0, 0.5] range.
		const double amps[], 			//!< [in] amplitude characteristic at each band edge [band_count * 2].
		const double weights[], 		//!< [in] error weights for each band [band_count].
		double h[], 					//!< [out] designed filter impulse response [order + 1].
		type::spec  = type::type_I_II, 	//!< [in] type of filter to design.
		unsigned grid_density = grid_density_default, 		//!< [in] initial grid density.
		unsigned max_iterations = max_iterations_default 	//!< [in] max iteration count.
		);

} // namespace pm

//! @brief FIR design using frequency sampling method
namespace fs {

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
DSPXX_API unsigned design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],			//!< [in] amplitude characteristic at each frequency point
		std::complex<double> H[] 	//!< [out] filter response designed in the spectrum domain [order + 1] or [nfft * 2] if nfft set.
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
DSPXX_API unsigned design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		const double wnd[],			//!< [in] window function to apply to the impulse response, needs to be of appropriate length. If explicitly set to NULL, no windowing will be used.
		double h[] 				//!< [out] designed filter impulse response [order + 1].
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
DSPXX_API unsigned design(
		unsigned order,				//!< [in] filter order, number of coefficients will be order + 1.
		unsigned point_count, 		//!< [in] number of points in the filter specification
		const double freqs[], 		//!< [in] frequency points in [0, 0.5] range
		const double amps[],		//!< [in] amplitude characteristic at each frequency point
		double h[] 					//!< [out] designed filter impulse response [order + 1].
		);
} // namespace fs

}} // namespace dsp::fir

#endif // DSP_FLT_FIR_DESIGN_H_INCLUDED