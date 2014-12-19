/*!
 * @file dsp++/zeropole.h
 * @brief Tools for converting between DF II, SOS & zero-pole model
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_ZEROPOLE_H_INCLUDED
#define DSP_ZEROPOLE_H_INCLUDED
#pragma once
#include <dsp++/export.h>
#include <complex>
#include <functional>

namespace dsp {

//! @brief Convert transfer function to zero-pole model.
//! Conjugate pair elements will always be adjacent to each other in the returned vectors.
//! @return gain coefficient k
DSPXX_API double tf2zp(
	unsigned bn,				//!< [in] number of numerator coefficients
	const double b[],			//!< [in] numerator coefficients [bn]
	unsigned an,				//!< [in] number of denominator coefficients, may be set to 0 with @p a set to NULL.
	const double a[],			//!< [in] denominator coefficients [an], may be NULL in which case single cefficient 1 will be used.
	unsigned& zn,				//!< [out] length of @p z vector
	std::complex<double> z[],	//!< [out] transfer function zeros, should have space for @p bn - 1 elements [bn - 1]
	unsigned& pn,				//!< [out] length of @p p vector
	std::complex<double> p[]	//!< [out] transfer function poles, should have space for @p an - 1 elements [an - 1], may be NULL if @p a is NULL.
);

}

#endif /* DSP_ZEROPOLE_H_INCLUDED */
