/*!
 * @file dsp++/dft.h
 * @brief Constants types etc common to various DFT implementations.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_DFT_H_INCLUDED
#define DSP_DFT_H_INCLUDED

namespace dsp { 

//! @brief Discrete Fourier Transform (DFT) tools
namespace dft {

//! @brief Constants specifying DFT transform direction/exponent sign
namespace sign {enum spec {
	/*!
	 * @brief Sign of exponent for forward DFT transform (-1).
	 * @see FFTW_FORWARD
	 */
	forward = -1,
	/*!
	 * @brief Sign of exponent for inverse DFT transform (1).
	 * @see FFTW_BACKWARD
	 */
	backward = 1
};}

}} // namespace dsp::dft

#endif /* DSP_DFT_H_INCLUDED */
