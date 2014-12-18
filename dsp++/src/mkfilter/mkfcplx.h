#pragma once
#include "mkfcplxt.h"
#include <cmath>

namespace mkfilter {

complex expj(double);

extern complex evaluate(complex[], int, complex[], int, const complex&);

inline double hypot(const complex& z) {return _hypot(z.imag(), z.real());}
inline double atan2(const complex& z) {return std::atan2(z.imag(), z.real()); }

inline complex sqr(const complex& z)
{ 
	return z * z;
}

} 