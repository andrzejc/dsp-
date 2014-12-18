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

//static double Xsqrt(double x)
//{ /* because of deficiencies in hypot on Sparc, it's possible for arg of Xsqrt to be small and -ve,
//  which logically it can't be (since r >= |x.re|).	 Take it as 0. */
//	return (x >= 0.0) ? sqrt(x) : 0.0;
//} 

complex mkfilter::expj(double theta)
{ 
	return complex(cos(theta), sin(theta));
}

