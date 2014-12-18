#pragma once
#include <complex>

namespace mkfilter {

struct c_complex { 
	double re, im;
};

struct complex { 
	double re, im;
	complex(double r, double i = 0.0) { re = r; im = i; }
	complex() { }					/* uninitialized complex */
	complex(c_complex z) { re = z.re; im = z.im; }	/* init from denotation */
	complex(const std::complex<double>& z) {re = z.real(); im = z.imag();}

	operator std::complex<double>() const {return std::complex<double>(re, im);}

};



}