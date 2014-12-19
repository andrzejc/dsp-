#include <dsp++/zeropole.h>
#include <dsp++/polyroots.h>
#include <dsp++/filter.h>
#include <cassert>
#include <vector>
#include <limits>

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
	zn = (bn > 1 ? dsp::roots(bn - 1, b, z) : 0);
	pn = (an > 1 ? dsp::roots(an - 1, a, p) : 0);
	return k;
}

namespace {

static const double EPS = 1e-15;

inline bool is_complex(const std::complex<double>& c) {return fabs(imag(c)) >= std::numeric_limits<double>::epsilon();}

inline bool is_conj_pair(const std::complex<double>& c1, const std::complex<double>& c2)
{
	bool res = (fabs(real(c1) - real(c2)) <= EPS) &&
		(fabs(imag(c1) + imag(c2)) <= EPS);
	double rd = fabs(real(c1) - real(c2));
	double id = fabs(imag(c1) + imag(c2));
	return res;
}

static bool next_zp(unsigned& i, const unsigned n, const std::complex<double> v[], const std::complex<double>*& v1, const std::complex<double>*& v2, std::vector<bool>& used)
{
	while (i != n && used[i])
		++i;

	if (n == i) {
		v1 = v2 = NULL;
		return false;
	}

	v1 = &v[i];
	used[i] = true;
	++i;

	bool cplx = is_complex(*v1);
	for (unsigned j = i; j != n; ++j) 
	{
		if (used[j]) 
		{
			if (j == i)
				++i;
			continue;
		}

		if (cplx && !is_conj_pair(*v1, v[j]))
			continue;

		if (j == i)
			++i;
		used[j] = true;
		v2 = &v[j];
		return true;
	}
	if (cplx)
		throw std::invalid_argument("missing conjugate pair");

	v2 = NULL;
	return true;
}

static void render_sos(const std::complex<double>* v1, const std::complex<double>* v2, double v[])
{
	v[0] = 1; 
	if (NULL == v2) 
	{
		assert(!is_complex(*v1));
		v[1] = -real(*v1);
	}
	else {
		assert(!is_complex(-*v2 - *v1));
		assert(!is_complex((-*v2) * (-*v1)));
		v[1] = real(-*v2 - *v1);
		v[2] = real((-*v2) * (-*v1));
	}
}

}

unsigned dsp::zp2sos(unsigned zn, const std::complex<double> z[], unsigned pn, const std::complex<double> p[], 
	double k, double num[],	double den[])
{
	const unsigned N = (std::max(zn, pn) + 1) / 2;
	if (NULL == num || NULL == den)
		return N;

	if (NULL == z || NULL == p)
		throw std::invalid_argument("zero or pole vector must not be NULL");

	std::vector<bool> z_used(zn, false);
	std::vector<bool> p_used(pn, false);

	unsigned sos = 0;
	unsigned zi = 0, pi = 0;
	while (zi != zn || pi != pn) {
		const std::complex<double> *z1, *z2, *p1, *p2;
		bool has_z = next_zp(zi, zn, z, z1, z2, z_used);
		bool has_p = next_zp(pi, pn, p, p1, p2, p_used);
		if (!has_z && !has_p)
			break;

		render_sos(z1, z2, &num[sos * sos_length]);
		render_sos(p1, p2, &den[sos * sos_length]);
		++sos;
	}
	assert(N == sos);
	double g = pow(k, 1. / sos);
	std::transform(num, num + sos * sos_length, num, std::bind2nd(std::multiplies<double>(), g));
	return N;
}

#if 0
#include <dsp++/filter_design.h>

namespace {

static bool test_zeropoly() {
	double num[] = {0.000152680443041306,0,-0.000763402215206528,0,0.00152680443041306,0,-0.00152680443041306,0,0.000763402215206528,0,-0.000152680443041306};
	double den[] = {1,-1.38612834449623,4.52324470528758,-4.41407679195703,7.54879819591143,-5.19603184727521,5.88039458112558,-2.67238428324105,2.13333907697761,-0.505227322643404,0.283586922576172};

	unsigned zn, pn;
	std::complex<double> z[10], p[10];
	double k = dsp::tf2zp(11, num, 11, den, zn, z, pn, p);

	double sos_n[15], sos_d[15];
	unsigned N = dsp::zp2sos(zn, z, pn, p, k, sos_n, sos_d);
	return 1.526804430413055e-04 == k;
}

static const bool test = test_zeropoly();

static bool test_zeropoly2() {
	std::complex<double> z[20], p[20];
	double fc[] = {100./44100, 300./44100};
	double k = dsp::iir::design(10, z, p, dsp::iir::resp::bandpass, fc);

	double sos_n[30], sos_d[30];
	unsigned N = dsp::zp2sos(20, z, 20, p, k, sos_n, sos_d);
	return true;
}

static const bool test2 = test_zeropoly2();
}
#endif 