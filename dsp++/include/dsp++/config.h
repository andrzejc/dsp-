/*!
 * @file dsp++/config.h
 * @brief Configuration macros to adjust availability of some features.
 */
#pragma once

#ifndef DSPXX_USE_CUSTOM_CONFIG
/*!
 * @brief Controls the inclusion of project-specific dsp_custom_config.h file which allows to refine
 * configuration options for a particular usage environment.
 */
#define DSPXX_USE_CUSTOM_CONFIG 0
#endif // DSPXX_USE_CUSTOM_CONFIG

#if DSPXX_USE_CUSTOM_CONFIG
#include "dsp++_config.h"
#endif // DSPXX_USE_CUSTOM_CONFIG

#if defined(__GNUC__)
#define DSPXX_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
#endif // __GNUC__

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
