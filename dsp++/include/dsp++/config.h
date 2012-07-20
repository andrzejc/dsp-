/*!
 * @file config.h
 * @brief Configuration macros to adjust availability of some features.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_CONFIG_H_INCLUDED
#define DSP_CONFIG_H_INCLUDED

#ifndef DSP_SNDFILE_DISABLED
//! @brief Set to 1 to disable audio reading/writing through libsndfile.
#define DSP_SNDFILE_DISABLED 	0
#endif

#ifndef DSP_FFTW_DISABLED
//! @brief Set to 1 to disable DFT support through libfftw3.
#define DSP_FFTW_DISABLED 		0
#endif

//! @brief Set to 1 if libfftwf is available
#define DSP_FFTW_HAVE_FLOAT 	1
//! @brief Set to 1 if libfftw is available
#define DSP_FFTW_HAVE_DOUBLE 	1
//! @brief Set to 1 if libfftwl is available
#define DSP_FFTW_HAVE_LONG_DOUBLE 	0
//! @brief Set to 1 if libfftwq is available
#define DSP_FFTW_HAVE_QUAD 	0



#endif /* DSP_CONFIG_H_INCLUDED */
