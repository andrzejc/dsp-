#include <dsp++/flt/iir_design.h>
#include <dsp++/zeropole.h>
#include <stdexcept>
#include <cmath>
#include <memory>
#include <cstring>
#include <algorithm>
#include <cassert>

#include "mkfilter/mkfilter.h"

using namespace dsp;

namespace {

static inline double verify_f(double f)
{
	if (f < 0 || f > .5)
		throw std::domain_error("dsp::iir::design(): normalized frequency outside [0, 0.5]");
	return f;
}

static unsigned init_mkfilter(mkfilter::context& ctx, unsigned order, unsigned type, const double* fc, const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	unsigned sz = order;
	const unsigned type_mask = 0x0003;
	switch (type & type_mask) {
	case iir::type::bessel: 
		ctx.options |= mkfilter::opt_be; 
		break;
	case iir::type::chebyshev: 
		ctx.options |= mkfilter::opt_ch; 
		ctx.chebrip = (NULL == cheb_rip ? 3. : *cheb_rip);
		break;
	case iir::type::butterworth: 
		ctx.options |= mkfilter::opt_bu; 
		break;
	default:
		throw std::invalid_argument("dsp::iir::design(): invalid filter type specification");
	}

	const unsigned char_mask = 0x01f0;
	switch (type & char_mask) {
	case iir::resp::lowpass: 
		ctx.options |= mkfilter::opt_lp | mkfilter::opt_a;
		if (NULL != fc)
			ctx.raw_alpha1 = verify_f(*fc);
		break;
	case iir::resp::highpass:
		ctx.options |= mkfilter::opt_hp | mkfilter::opt_a;
		if (NULL != fc)
			ctx.raw_alpha1 = verify_f(*fc);
		break;
	case iir::resp::bandpass:
		ctx.options |= mkfilter::opt_bp | mkfilter::opt_a;
		if (NULL != fc) {
			ctx.raw_alpha1 = verify_f(fc[0]);
			ctx.raw_alpha2 = verify_f(fc[1]);
			if (ctx.raw_alpha1 > ctx.raw_alpha2)
				throw std::logic_error("dsp::iir::design(): band corner frequencies non-monotonic");
		}
		sz = 2 * order;
		break;
	case iir::resp::bandstop:
		ctx.options |= mkfilter::opt_bs | mkfilter::opt_a;
		if (NULL != fc) {
			ctx.raw_alpha1 = verify_f(fc[0]);
			ctx.raw_alpha2 = verify_f(fc[1]);
			if (ctx.raw_alpha1 > ctx.raw_alpha2)
				throw std::logic_error("dsp::iir::design(): band corner frequencies non-monotonic");
		}
		sz = 2 * order;
		break;
	case iir::resp::allpass:
		ctx.options |= mkfilter::opt_ap | mkfilter::opt_a;
		if (NULL != fc)
			ctx.raw_alpha1 = verify_f(*fc);
		break;
	default:
		throw std::invalid_argument("dsp::iir::design(): invalid filter characteristics specification");
	}

	if (NULL != zero_freq) {
		ctx.options |= mkfilter::opt_Z;
		ctx.raw_alphaz = verify_f(*zero_freq);
		sz += 2;
	}
	if (0 != pole_mask) {
		ctx.options |= mkfilter::opt_p;
		ctx.polemask = pole_mask;
	}
	if (type & iir::flag::prewrap)
		ctx.options |= mkfilter::opt_w;
	if (type & iir::flag::matched_z)
		ctx.options |= mkfilter::opt_z;

	ctx.order = static_cast<int>(order);
	ctx.options |= mkfilter::opt_o;
	return sz;
}

}


unsigned dsp::iir::design(unsigned order, unsigned type, const double* fc, 
	double b[], double a[], 
	const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	mkfilter::context ctx;
	memset(&ctx, 0, sizeof(ctx));
	unsigned sz = init_mkfilter(ctx, order, type, fc, cheb_rip, zero_freq, pole_mask);
	if (NULL == b || NULL == a)
		return sz + 1;

	ctx.xcoeffs = b;
	ctx.ycoeffs = a;

	mkfilter::design(ctx, sz);

	assert(ctx.splane.numzeros <= (int)sz);
	assert(ctx.splane.numpoles <= (int)sz);
	assert(ctx.zplane.numzeros == sz);
	assert(ctx.zplane.numpoles == sz);
	return sz + 1;
}

double dsp::iir::design(unsigned order, unsigned type, const double* fc, 
	std::complex<double> z[], std::complex<double> p[], 
	const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	mkfilter::context ctx;
	memset(&ctx, 0, sizeof(ctx));
	unsigned sz = init_mkfilter(ctx, order, type, fc, cheb_rip, zero_freq, pole_mask);

	mkfilter::design(ctx, sz);

	assert(ctx.splane.numzeros <= (int)sz);
	assert(ctx.splane.numpoles <= (int)sz);
	assert(ctx.zplane.numzeros == sz);
	assert(ctx.zplane.numpoles == sz);
	std::copy(ctx.zplane.zeros, ctx.zplane.zeros + sz, z);
	std::copy(ctx.zplane.poles, ctx.zplane.poles + sz, p);
	return 1/gain(ctx);
}

DSPXX_API unsigned dsp::iir::sos_design(unsigned order, unsigned type, const double* fc,
	double num[], double den[], const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	mkfilter::context ctx;
	memset(&ctx, 0, sizeof(ctx));
	unsigned sz = init_mkfilter(ctx, order, type, fc, cheb_rip, zero_freq, pole_mask);

	if (NULL == num || NULL == den)
		return zp2sos(sz, NULL, sz, NULL, 0., NULL, NULL);

	mkfilter::design(ctx, sz);

	assert(ctx.splane.numzeros <= (int)sz);
	assert(ctx.splane.numpoles <= (int)sz);
	assert(ctx.zplane.numzeros == sz);
	assert(ctx.zplane.numpoles == sz);

	double k = 1/gain(ctx);
	return zp2sos(ctx.zplane.numzeros, ctx.zplane.zeros, ctx.zplane.numpoles, ctx.zplane.poles, k, num, den);
}



#if 0
namespace {

static bool test_iir() {
	//double b[21], a[21];
	double fc[2] = {100./44100, 300./44100};
	//iir_filter_design(10, b, a, dsp::iir_bandpass, fc);

	std::complex<double> z[20], p[20];
	double k = iir::design(4, z, p, iir::resp::bandstop, fc);
	return true;
}

static const bool test = test_iir();

}
#endif
