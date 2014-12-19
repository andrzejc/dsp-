#pragma once
#include "mkfcplxt.h"
#include <cmath>

namespace mkfilter {

complex expj(double);

extern complex evaluate(complex[], int, complex[], int, const complex&);

complex evaluate_zp(complex zeros[], int nz, complex poles[], int np, const complex& z);

double hypot(const complex& z);

inline double atan2(const complex& z) {return std::atan2(z.imag(), z.real()); }

inline complex sqr(const complex& z)
{ 
	return z * z;
}

} 