#pragma once
#include <complex>

namespace mkfilter {

struct c_complex { 
	double re, im;
};

typedef std::complex<double> complex;

}