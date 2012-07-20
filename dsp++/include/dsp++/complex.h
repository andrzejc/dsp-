/*!
 * @file dsp++/complex.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_COMPLEX_H_INCLUDED
#define DSP_COMPLEX_H_INCLUDED

#include <complex>

namespace dsp {

template<class Real>
Real conj(Real real) {return real;}

template<class Real>
std::complex<Real> conj(const std::complex<Real>& cplx) {return std::conj(cplx);}

template<class Real> struct remove_complex {typedef Real type;};
template<class Real> struct remove_complex<std::complex<Real> > {typedef Real type;};
template<class Real> struct remove_complex<std::complex<Real> const> {typedef Real type;};
template<class Real> struct remove_complex<std::complex<Real> volatile> {typedef Real type;};
template<class Real> struct remove_complex<std::complex<Real> const volatile> {typedef Real type;};

}

#endif /* DSP_COMPLEX_H_INCLUDED */
