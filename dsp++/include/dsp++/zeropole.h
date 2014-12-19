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

//! @brief Convert filter transfer function to zero-pole representation.
//! Conjugate pair elements will always be adjacent to each other in the returned vectors.
//! @return system gain k
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

//! @brief Convert zero-pole filter representation to second-order-sections.
//! @return number of second-order-section written, or going to be written to @p num, @p den (ceil(max(zn, pn) / 2)); needs to be multiplied by @p sos_length to get @p num/@p den length
//! @throw std::invalid_argument if @p z or @p p are NULL with non-null @p num or @p den
DSPXX_API unsigned zp2sos(
	unsigned zn,					//!< [in] length of @p z (zeros) vector
	const std::complex<double> z[],	//!< [in] zeros vector (zn), may be NULL if @p num and @p den set to NULL
	unsigned pn,					//!< [in] length of @p p (poles) vector
	const std::complex<double> p[],	//!< [in] poles vector (pn), may be NULL if @p num and @p den set to NULL
	double k,						//!< [in] system gain
	double num[],					//!< [out] second-order-sections numerators (3 * ceil(max(zn, pn) / 2)), if NULL, this function will only return the number of sections
	double den[]					//!< [out] second-order-sections denominators (3 * ceil(max(zn, pn) / 2)), if NULL, this function will only return the number of sections
);

}

#endif /* DSP_ZEROPOLE_H_INCLUDED */
