/* mkfilter -- given n, compute recurrence relation
to implement Butterworth, Bessel or Chebyshev filter of order n
A.J. Fisher, University of York   <fisher@minster.york.ac.uk>
September 1992 */

/* Header file */

namespace mkfilter {

#define unless(x)   if (!(x))
#define until(x)    while (!(x))

#undef	PI
#define PI	    3.14159265358979323846  /* Microsoft C++ does not define M_PI ! */
#define TWOPI	    (2.0 * PI)
#define EPS	    1e-10
	//#define MAXORDER    10
	//#define mkfilter::max_pz	    512	    /* .ge. 2*MAXORDER, to allow for doubling of poles in BP filter;
	//			       high values needed for FIR filters */

inline double sqr(double x)	    {return x*x;}
inline bool onebit(unsigned m)	    {return (m != 0) && ((m & (m-1)) == 0);}

inline double asinh(double x)
{ /* Microsoft C++ does not define */
	return log(x + sqrt(1.0 + sqr(x)));
}

inline double fix(double x)
{ /* nearest integer */
	return (x >= 0.0) ? floor(0.5+x) : -floor(0.5-x);
}

}
