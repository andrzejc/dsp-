/*!
 * @file config.h
 * @brief Configuration macros to adjust availability of some features.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_CONFIG_H_INCLUDED
#define DSP_CONFIG_H_INCLUDED
#pragma once

#ifndef DSP_USE_CUSTOM_CONFIG
/*!
 * @brief Controls the inclusion of project-specific dsp_custom_config.h file which allows to refine
 * configuration options for a particular usage environment.
 */
#define DSP_USE_CUSTOM_CONFIG 0
#endif // DSP_USE_CUSTOM_CONFIG

#if DSP_USE_CUSTOM_CONFIG
#include "dsp_custom_config.h"
#endif // DSP_USE_CUSTOM_CONFIG

#define DSP_CONFORM_CPP0X (__cplusplus >= 201103L)

#ifndef DSP_SNDFILE_DISABLED
//! @brief Set to 1 to disable audio reading/writing through libsndfile.
#define DSP_SNDFILE_DISABLED 	0
#endif // DSP_SNDFILE_DISABLED

#ifndef DSP_FFTW_DISABLED
//! @brief Set to 1 to disable DFT support through libfftw3.
#define DSP_FFTW_DISABLED 		0
#endif // DSP_FFTW_DISABLED

#ifndef DSP_BOOST_DISABLED
//! @brief Set to 1 to disable use of boost libraries entirely.
#define DSP_BOOST_DISABLED		0
#endif // DSP_BOOST_DISABLED

#ifndef DSP_BOOST_CONCEPT_CHECKS_DISABLED
//! @brief Set to 1 to disable use of Boost Concept Check Library {@see http://www.boost.org/libs/concept_check/}
#define DSP_BOOST_CONCEPT_CHECKS_DISABLED (DSP_BOOST_DISABLED)
#endif // DSP_BOOST_CONCEPT_CHECKS_DISABLED

#ifndef DSP_FFTW_HAVE_FLOAT
//! @brief Set to 1 if libfftwf is available
#define DSP_FFTW_HAVE_FLOAT 	1
#endif // DSP_FFTW_HAVE_FLOAT

#ifndef DSP_FFTW_HAVE_DOUBLE
//! @brief Set to 1 if libfftw is available
#define DSP_FFTW_HAVE_DOUBLE 	1
#endif // DSP_FFTW_HAVE_DOUBLE

#ifndef DSP_FFTW_HAVE_LONG_DOUBLE
//! @brief Set to 1 if libfftwl is available
#define DSP_FFTW_HAVE_LONG_DOUBLE 	1
#endif // DSP_FFTW_HAVE_LONG_DOUBLE

#ifndef DSP_FFTW_HAVE_QUAD
//! @brief Set to 1 if libfftwq is available
#define DSP_FFTW_HAVE_QUAD 	0
#endif // DSP_FFTW_HAVE_QUAD

#endif /* DSP_CONFIG_H_INCLUDED */
