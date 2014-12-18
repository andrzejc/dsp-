#include <dsp++/zeropole.h>
#include <dsp++/polyroots.h>
#include <cassert>

double dsp::tf2zp(unsigned bn, const double b[], unsigned an, const double a[], unsigned& zn, std::complex<double> z[], unsigned& pn, std::complex<double> p[])
{
	assert(NULL != b);
	double aa = 1.;
	if (NULL == a) {
		an = 1;
		a = &aa;
	}

	while (bn != 0 && 0. == *b) {
		++b;
		--bn;
	}
	if (0 == bn) 
		throw std::invalid_argument("numerator vector must not be empty");

	double k = *b / *a;
	zn = (bn > 1 ? dsp::rpoly(bn - 1, b, z) : 0);
	pn = (an > 1 ? dsp::rpoly(an - 1, a, p) : 0);
	return k;
}


#if 0
namespace {

static bool test_zeropoly() {
	double num[] = {0.000152680443041306,0,-0.000763402215206528,0,0.00152680443041306,0,-0.00152680443041306,0,0.000763402215206528,0,-0.000152680443041306};
	double den[] = {1,-1.38612834449623,4.52324470528758,-4.41407679195703,7.54879819591143,-5.19603184727521,5.88039458112558,-2.67238428324105,2.13333907697761,-0.505227322643404,0.283586922576172};

	unsigned zn, pn;
	std::complex<double> z[10], p[10];
	double k = dsp::tf2zp(11, num, 11, den, zn, z, pn, p);
	return 1.526804430413055e-04 == k;
}

static const bool test = test_zeropoly();

}
#endif 