/*!
 * @file dsp++/dft.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_DFT_H_INCLUDED
#define DSP_DFT_H_INCLUDED

namespace dsp {

/*!
 * @brief Sign of exponent for forward DFT transform (-1).
 * @see FFTW_FORWARD
 */
const int dft_sign_forward = -1;
/*!
 * @brief Sign of exponent for inverse DFT transform (1).
 * @see FFTW_BACKWARD
 */
const int dft_sign_backward = 1;

}

#endif /* DSP_DFT_H_INCLUDED */
