/*!
 * @file dsp++/complex.h
 * @brief Utilities dealing with complex numbers.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_COMPLEX_H_INCLUDED
#define DSP_COMPLEX_H_INCLUDED

#include <complex>

namespace dsp {

using std::complex;

/*!
 * @brief Extended version of std::conj() which works also for non-complex types, useful for building generic
 * algorithms dealing with either complex or real numbers (for real number it's simply an identity transform).
 * @param real number
 * @return the number
 */
template<class Real>
Real conj(Real real) {return real;}

/*!
 * @brief Specialization of dsp::conj() for complex numbers which actually returns std::conj().
 * @param cplx number
 * @return conjugate of number.
 */
template<class Real>
std::complex<Real> conj(const std::complex<Real>& cplx) {return std::conj(cplx);}


/*!
 * @brief A type-traits approach to obtaining base (real) type of complex type, for constructing
 * generic algorithms working on complex as well as on real numbers.
 */
template<class Real> struct remove_complex {typedef Real type;};
//! @copydoc remove_complex
template<class Real> struct remove_complex<std::complex<Real> > {typedef Real type;};
//! @copydoc remove_complex
template<class Real> struct remove_complex<std::complex<Real> const> {typedef Real type;};
//! @copydoc remove_complex
template<class Real> struct remove_complex<std::complex<Real> volatile> {typedef Real type;};
//! @copydoc remove_complex
template<class Real> struct remove_complex<std::complex<Real> const volatile> {typedef Real type;};

}

#endif /* DSP_COMPLEX_H_INCLUDED */
