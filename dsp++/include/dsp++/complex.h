/// @file dsp++/complex.h
/// @brief Utilities dealing with complex numbers.
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_COMPLEX_H_INCLUDED
#define DSP_COMPLEX_H_INCLUDED

#include <complex>

namespace dsp {

using std::complex;

/// @brief Extended version of std::conj() which works also for non-complex types, useful for building generic
/// algorithms dealing with either complex or real numbers (for real number it's simply an identity transform).
/// @param real some number.
/// @return the conjugate of a real number, that is the number itself.
template<class Real>
inline Real conj(Real real) {return real;}

/// @brief Specialization of dsp::conj() for complex numbers which actually returns std::conj().
/// @param cplx number
/// @return conjugate of number.
template<class Real>
inline complex<Real> conj(const complex<Real>& cplx) {return std::conj(cplx);}

/// @brief Extended version of std::real() which works also for non-complex types, useful for building generic algorithms.
/// @param real some number.
/// @return the real part of a real number, that is the number itself.
template<class Real>
inline Real real(Real real) {return real;}

template<class Real>
inline complex<Real> real(const complex<Real>& cplx) {return std::real(cplx);}

/// @brief Extended version of std::imag() which works also for non-complex types, useful for building generic algorithms.
/// @param real some number.
/// @return the imaginary part of a real number, that is 0.
template<class Real>
inline Real imag(Real real) {return Real();}

template<class Real>
inline complex<Real> imag(const complex<Real>& cplx) {return std::imag(cplx);}

/// @brief A type-traits approach to obtaining base (real) type of complex type, for constructing
/// generic algorithms working on complex as well as on real numbers.
template<class Real> struct remove_complex {typedef Real type;};
/// @copydoc remove_complex
template<class Real> struct remove_complex<complex<Real> > {typedef Real type;};
/// @copydoc remove_complex
template<class Real> struct remove_complex<complex<Real> const> {typedef Real type;};
/// @copydoc remove_complex
template<class Real> struct remove_complex<complex<Real> volatile> {typedef Real type;};
/// @copydoc remove_complex
template<class Real> struct remove_complex<complex<Real> const volatile> {typedef Real type;};

}

#endif /* DSP_COMPLEX_H_INCLUDED */
