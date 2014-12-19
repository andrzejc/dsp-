/* mkfilter -- given n, compute recurrence relation
to implement Butterworth, Bessel or Chebyshev filter of order n
A.J. Fisher, University of York   <fisher@minster.york.ac.uk>
September 1992 */

/* Routines for complex arithmetic */

#include <cmath>
#include <cstring>

#include "mkfilter.h"
#include "mkfcplx.h"

using namespace mkfilter;

static complex eval(complex[], int, const complex&);
static double Xsqrt(double);


static complex eval(complex coeffs[], int npz, const complex& z)
{ /* evaluate polynomial in z, substituting for z */
	complex sum(0);
	for (int i = npz; i >= 0; i--) 
		sum = (sum * z) + coeffs[i];
	return sum;
}

complex mkfilter::evaluate(complex topco[], int nz, complex botco[], int np, const complex& z)
{ /* evaluate response, substituting for z */
	return eval(topco, nz, z) / eval(botco, np, z);
}

namespace {

static mkfilter::complex cdivid(const complex& a, const complex& b)
{
	double r, d, cr, ci, infin = std::numeric_limits<double>::max();

	if (complex() == b)
		return a / b;

	if( fabs(real(b)) < fabs(imag(b)) )
	{
		r = real(b) / imag(b);
		d = imag(b) + r * real(b);
		cr = ( real(a) * r + imag(a) ) / d;
		ci = ( imag(a) * r - real(a) ) / d;
	}
	else 
	{
		r = imag(b) / real(b);
		d = real(b) + r * imag(b);
		cr = ( real(a) + imag(a) * r ) / d;
		ci = ( imag(a) - real(a) * r ) / d;
	}
	return complex(cr, ci);
}

}

complex mkfilter::evaluate_zp(complex zeros[], int nz, complex poles[], int np, const complex& z)
{ /* evaluate response, substituting for z */
	complex res = 1;
	const int nzp = std::min(nz, np);
	for (int i = 0; i < nzp; ++i) 
		res *= cdivid(z - zeros[i], z - poles[i]);
	for (int i = nzp; i < nz; ++i)
		res *= (z - zeros[i]);
	for (int i = nzp; i < np; ++i)
		res = cdivid(res, z - poles[i]);
	return res;
}

//static double Xsqrt(double x)
//{ /* because of deficiencies in hypot on Sparc, it's possible for arg of Xsqrt to be small and -ve,
//  which logically it can't be (since r >= |x.re|).	 Take it as 0. */
//	return (x >= 0.0) ? sqrt(x) : 0.0;
//} 

complex mkfilter::expj(double theta)
{ 
	return complex(cos(theta), sin(theta));
}

double mkfilter::hypot(const complex& z)
{
	double ar, ai;

	ar = fabs(real(z));
	ai = fabs(imag(z));
	if( ar < ai )
		return ai * sqrt( 1.0 + pow( ( ar / ai ), 2.0 ) );

	if( ar > ai )
		return ar * sqrt( 1.0 + pow( ( ai / ar ), 2.0 ) );

	return ar * sqrt( 2.0 );
}

