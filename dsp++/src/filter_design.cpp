/*!
 * @file filter_design.cpp
 * @brief Implementation of filter design algorithms.
 */
#include <dsp++/filter_design.h>
#include "remez/remez.h"
#include "mkfilter/mkfilter.h"

#include <stdexcept>
#include <cmath>
#include <memory>
#include <cstring>
#include <algorithm>
#include <cassert>

#include <dsp++/const.h>

using namespace dsp;

bool dsp::firpm(size_t order, double h[], size_t band_count, double freqs[], const double amps[],
		const double weights[], filter_type type, size_t grid_density, size_t max_iterations)
{
	remez_filter_type ft;
	switch (type) {
	case filter_type_differentiator: ft = REMEZ_FILTER_DIFFERENTIATOR; break;
	case filter_type_hilbert: ft = REMEZ_FILTER_HILBERT; break;
	default: ft = REMEZ_FILTER_BANDPASS; break;
	}
	double lf = 0.;
	for (size_t i = 0; i < 2 * band_count; ++i)
	{
		double f = freqs[i];
		if (f < 0. || f > 0.5)
			throw std::domain_error("dsp::firpm(): freqs outside [0, 0.5]");
		if (f < lf)
			throw std::invalid_argument("dsp::firpm(): freqs non-monotonic");
		lf = f;
	}
	for (size_t i = 0; i < band_count; ++i)
		if (weights[i] <= 0.)
			throw std::domain_error("dsp::firpm(): weights <= 0");

	int res = remez(h, static_cast<int>(order + 1), static_cast<int>(band_count), freqs, amps, weights, ft, static_cast<int>(grid_density), static_cast<int>(max_iterations));
	if (REMEZ_FAILED(res))
		throw std::bad_alloc();

	return (REMEZ_NOERR == res);
}

void dsp::biquad::design(double b[], double a[], biquad::type::spec type, double norm_freq, const double* gain_db, const double* q, const double* bw, const double* s)
{
	if (norm_freq < 0. || norm_freq > 0.5)
		throw std::domain_error("dsp::biquad_design(): norm_freq outside [0, 0.5]");

	double w0 = DSP_M_PI * 2. * norm_freq; 	// 2 * pi * f0 / Fs
	double cw0 = std::cos(w0);

	double A = 0, 	// sqrt(10^(gain_db/20))
		alpha,
		Am1 = 0,	// A - 1
		Ap1 = 0, 	// A + 1
		Am1c = 0, 	// (A-1)*cos(w0)
		Ap1c = 0, 	// (A+1)*cos(w0)
		sqAa2 = 0;	// 2 * sqrt(A) * alpha
	bool is_eq;
	switch (type) {
	case biquad::type::peaking_eq:
	case biquad::type::low_shelf_eq:
	case biquad::type::high_shelf_eq:
		if (NULL == gain_db)
			throw std::invalid_argument("dsp::biquad_design(): gain_db not specified");
		A = std::pow(10., *gain_db / 40.);
		Am1 = A - 1; Ap1 = A + 1; Am1c = Am1 * cw0; Ap1c = Ap1 * cw0;
		is_eq = true;
		break;
	default:
		is_eq = false;
		break;
	}

	double sw0 = std::sin(w0);
	if (NULL != q)
		alpha = sw0 / (2 * (*q));
	else if (NULL != bw)
		alpha = sw0 * std::sinh(DSP_M_LN2 / 2 * (*bw) * w0/sw0);
	else if (is_eq && NULL != s)
		alpha = sw0 / 2 * std::sqrt((A + 1/A) * (1/(*s) -1) + 2);
	else
		throw std::invalid_argument("dsp::biquad_design(): q, bw, s not specified");

	if (is_eq)
		sqAa2 = 2 * std::sqrt(A) * alpha;

	double n2c = -2 * cw0;	// -2 * cos(w0)
	switch (type) {
	case biquad::type::lowpass:
		b[0] = (1 - cw0) / 2; b[1] = 1 - cw0; b[2] = b[0];
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad::type::highpass:
		b[0] = (1 + cw0) / 2; b[1] = -(1 + cw0); b[2] = b[0];
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad::type::bandpass:
		b[0] = alpha; b[1] = 0; b[2] = -alpha;
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad::type::notch:
		b[0] = 1; b[1] = n2c; b[2] = 1;
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad::type::allpass:
		b[0] = 1 - alpha; b[1] = n2c; b[2] = 1 + alpha;
		a[0] = b[2]; a[1] = n2c; a[2] = b[0];
		break;
	case biquad::type::peaking_eq:
		b[0] = 1 + alpha * A; b[1] = n2c; b[2] = 1 - alpha * A;
		a[0] = 1 + alpha / A; a[1] = n2c; a[2] = 1 - alpha / A;
		break;
	case biquad::type::low_shelf_eq:
		b[0] = A * (Ap1 - Am1c + sqAa2); b[1] = 2 * A * (Am1 - Ap1c); b[2] = A * (Ap1 - Am1c - sqAa2);
		a[0] = (Ap1 + Am1c + sqAa2); a[1] = -2 * (Am1 + Ap1c); a[2] = Ap1 + Am1c - sqAa2;
		break;
	case biquad::type::high_shelf_eq:
		b[0] = A * (Ap1 + Am1c + sqAa2); b[1] = -2 * A * (Am1 + Ap1c); b[2] = A * (Ap1 + Am1c - sqAa2);
		a[0] = (Ap1 - Am1c + sqAa2); a[1] = 2 * (Am1 - Ap1c); a[2] = Ap1 - Am1c - sqAa2;
		break;
	}
}

namespace {

static size_t init_mkfilter(mkfilter::context& ctx, size_t order, unsigned type, const double* fc, const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	size_t sz = order;
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
		throw std::invalid_argument("invalid filter type specification");
	}

	const unsigned char_mask = 0x01f0;
	switch (type & char_mask) {
	case iir::resp::lowpass: 
		ctx.options |= mkfilter::opt_lp | mkfilter::opt_a;
		ctx.raw_alpha1 = *fc;
		break;
	case iir::resp::highpass:
		ctx.options |= mkfilter::opt_hp | mkfilter::opt_a;
		ctx.raw_alpha1 = *fc;
		break;
	case iir::resp::bandpass:
		ctx.options |= mkfilter::opt_bp | mkfilter::opt_a;
		ctx.raw_alpha1 = fc[0];
		ctx.raw_alpha2 = fc[1];
		sz = 2 * order;
		break;
	case iir::resp::bandstop:
		ctx.options |= mkfilter::opt_bs | mkfilter::opt_a;
		ctx.raw_alpha1 = fc[0];
		ctx.raw_alpha2 = fc[1];
		sz = 2 * order;
		break;
	case iir::resp::allpass:
		ctx.options |= mkfilter::opt_ap | mkfilter::opt_a;
		ctx.raw_alpha1 = *fc;
		break;
	default:
		throw std::invalid_argument("missing filter type specification");
	}

	if (NULL != zero_freq) {
		ctx.options |= mkfilter::opt_Z;
		ctx.raw_alphaz = *zero_freq;
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

	ctx.splane.init(sz);
	ctx.zplane.init(sz);
	return sz;
}

}


void dsp::iir::design(size_t order, double b[], double a[], unsigned type, const double* fc, const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	mkfilter::context ctx;
	memset(&ctx, 0, sizeof(ctx));
	size_t sz = init_mkfilter(ctx, order, type, fc, cheb_rip, zero_freq, pole_mask);
	ctx.xcoeffs = b;
	ctx.ycoeffs = a;

	mkfilter::design(ctx);

	assert(ctx.splane.numzeros <= (int)sz);
	assert(ctx.splane.numpoles <= (int)sz);
	assert(ctx.zplane.numzeros == sz);
	assert(ctx.zplane.numpoles == sz);
	// inverse coeffs order - mkfilter produces vectors in the format for its own internal filter implementation which uses inversed vectors
	for (size_t i = 0; i < (sz + 1) / 2; ++i) 
	{
		std::swap(b[i], b[sz - i]);
		std::swap(a[i], a[sz - i]);
	}
}

double dsp::iir::design(size_t order,	std::complex<double> z[], std::complex<double> p[],	unsigned type, 
	const double* fc, const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	mkfilter::context ctx;
	memset(&ctx, 0, sizeof(ctx));
	size_t sz = init_mkfilter(ctx, order, type, fc, cheb_rip, zero_freq, pole_mask);

	mkfilter::design(ctx);

	assert(ctx.splane.numzeros <= (int)sz);
	assert(ctx.splane.numpoles <= (int)sz);
	assert(ctx.zplane.numzeros == sz);
	assert(ctx.zplane.numpoles == sz);
	std::copy(ctx.zplane.zeros, ctx.zplane.zeros + sz, z);
	std::copy(ctx.zplane.poles, ctx.zplane.poles + sz, p);
	return 1/gain(ctx);
}


#if 1
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
