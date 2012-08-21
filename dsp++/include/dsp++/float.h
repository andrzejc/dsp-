/*!
 * @file dsp++/float.h
 * @brief Various helper routines dealing with float numbers.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_FLOAT_H_INCLUDED
#define DSP_FLOAT_H_INCLUDED

#include <cmath>
#include <functional>
#include <limits>
#include <complex>

namespace dsp {

/*!
 * @brief Test whether floating-point numbers are within +/-epsilon range from each other.
 * @param lhs left-hand side number.
 * @param rhs right-hand side number.
 * @return @c true if the numbers are roughly equal, @c false otherwise.
 */
template<class Real>
struct within_range: public std::binary_function<Real, Real, bool>
{
	const Real margin;
	explicit within_range(Real mrg = std::numeric_limits<Real>::epsilon()): margin(mrg) {}

	static bool value(Real lhs, Real rhs, Real epsilon)
	{
		return !std::isnan(lhs) && !std::isnan(rhs) &&
				(rhs >= (lhs - epsilon)) &&
				(rhs <= (lhs + epsilon));
	}

	bool operator()(Real lhs, Real rhs) const {return value(lhs, rhs, margin);}

};

template<class Real>
struct within_range<std::complex<Real> >: public std::binary_function<std::complex<Real>, std::complex<Real>, bool>
{
	const Real margin;
	explicit within_range(Real mrg = std::numeric_limits<Real>::epsilon()): margin(mrg) {}

	bool operator()(const std::complex<Real>& lhs, const std::complex<Real>& rhs) const
	{
		return within_range<Real>::value(lhs.real(), rhs.real(), margin) &&
				within_range<Real>::value(lhs.imag(), rhs.imag(), margin);
	}
};

template<class Real>
struct differs_by: public std::binary_function<Real, Real, bool>
{
	const Real factor;
	explicit differs_by(Real fctr = std::numeric_limits<Real>::epsilon()): factor(fctr) {}

	static bool value(Real lhs, Real rhs, Real factor)
	{
		return !std::isnan(lhs) && !std::isnan(rhs) &&
				(rhs >= (lhs * (Real(1) - factor))) &&
				(rhs <= (lhs * (Real(1) + factor)));
	}

	bool operator()(Real lhs, Real rhs) const {return value(lhs, rhs, factor);}
};

template<class Real>
struct differs_by<std::complex<Real> >: public std::binary_function<std::complex<Real>, std::complex<Real>, bool>
{
	const Real factor;
	explicit differs_by(Real fctr = std::numeric_limits<Real>::epsilon()): factor(fctr) {}

	bool operator()(const std::complex<Real>& lhs, const std::complex<Real>& rhs) const
	{
		return within_range<Real>::value(lhs.real(), rhs.real(), factor) &&
				within_range<Real>::value(lhs.imag(), rhs.imag(), factor);
	}
};

}

#endif /* DSP_FLOAT_H_INCLUDED */