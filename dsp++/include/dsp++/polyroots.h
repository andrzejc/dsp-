/*!
 * @file dsp++/polyroots.h
 * @brief Polynomial root solver
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_POLYROOTS_H_INCLUDED
#define DSP_POLYROOTS_H_INCLUDED
#pragma once
#include <dsp++/export.h>
#include <complex>

namespace dsp {

//! @brief Solve polynomial for roots using rpoly algorithm.
//! @return number of roots returned in @p roots vector.
DSPXX_API unsigned roots(
	unsigned degree,				//!< [in] polynomial degree (highest exponent of the variable).
	const double poly[],			//!< [in] polynomial coefficients (degree + 1)
	std::complex<double> roots[]	//!< [out] roots of the polynomial, with space for at least (degree) elements
);

DSPXX_API double polyeval(
	unsigned degree,
	const double poly[]
);

}

#endif /* DSP_POLYROOTS_H_INCLUDED */
