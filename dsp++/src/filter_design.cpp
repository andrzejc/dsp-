/*!
 * @file filter_design.cpp
 * @brief Implementation of filter design algorithms.
 */
#include <dsp++/filter_design.h>
#include "remez/remez.h"
#include "mkfilter/mkfltif.h"

#include <stdexcept>
#include <cmath>
#include <memory>
#include <cstring>
#include <algorithm>

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

void dsp::biquad_design(double b[], double a[], biquad_type type, double norm_freq, const double* gain_db, const double* q, const double* bw, const double* s)
{
	if (norm_freq < 0. || norm_freq > 0.5)
		throw std::domain_error("dsp::biquad_design(): norm_freq outside [0, 0.5]");

	double w0 = DSP_M_PI * 2. * norm_freq; 	// 2 * pi * f0 / Fs
	double cw0 = std::cos(w0);

	double A, 	// sqrt(10^(gain_db/20))
		alpha,
		Am1,	// A - 1
		Ap1, 	// A + 1
		Am1c, 	// (A-1)*cos(w0)
		Ap1c, 	// (A+1)*cos(w0)
		sqAa2;	// 2 * sqrt(A) * alpha
	bool is_eq;
	switch (type) {
	case biquad_type_peaking_eq:
	case biquad_type_low_shelf_eq:
	case biquad_type_high_shelf_eq:
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
	case biquad_type_lowpass:
		b[0] = (1 - cw0) / 2; b[1] = 1 - cw0; b[2] = b[0];
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad_type_highpass:
		b[0] = (1 + cw0) / 2; b[1] = -(1 + cw0); b[2] = b[0];
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad_type_bandpass:
		b[0] = alpha; b[1] = 0; b[2] = -alpha;
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad_type_notch:
		b[0] = 1; b[1] = n2c; b[2] = 1;
		a[0] = 1 + alpha; a[1] = n2c; a[2] = 1 - alpha;
		break;
	case biquad_type_allpass:
		b[0] = 1 - alpha; b[1] = n2c; b[2] = 1 + alpha;
		a[0] = b[2]; a[1] = n2c; a[2] = b[0];
		break;
	case biquad_type_peaking_eq:
		b[0] = 1 + alpha * A; b[1] = n2c; b[2] = 1 - alpha * A;
		a[0] = 1 + alpha / A; a[1] = n2c; a[2] = 1 - alpha / A;
		break;
	case biquad_type_low_shelf_eq:
		b[0] = A * (Ap1 - Am1c + sqAa2); b[1] = 2 * A * (Am1 - Ap1c); b[2] = A * (Ap1 - Am1c - sqAa2);
		a[0] = (Ap1 + Am1c + sqAa2); a[1] = -2 * (Am1 + Ap1c); a[2] = Ap1 + Am1c - sqAa2;
		break;
	case biquad_type_high_shelf_eq:
		b[0] = A * (Ap1 + Am1c + sqAa2); b[1] = -2 * A * (Am1 + Ap1c); b[2] = A * (Ap1 + Am1c - sqAa2);
		a[0] = (Ap1 - Am1c + sqAa2); a[1] = 2 * (Am1 - Ap1c); a[2] = Ap1 - Am1c - sqAa2;
		break;
	}
}


void dsp::iir_filter_design(size_t order, double b[], double a[], unsigned type, const double* fc, const double* cheb_rip, const double* zero_freq, unsigned pole_mask)
{
	std::auto_ptr<mkfilter::inout> io(new mkfilter::inout);
	memset(io.get(), 0, sizeof(mkfilter::inout));
	size_t sz = order;

	const unsigned type_mask = 0x0003;
	switch (type & type_mask) {
	case iir_bessel: 
		io->options |= mkfilter_opt_be; 
		break;
	case iir_chebyshev: 
		io->options |= mkfilter_opt_ch; 
		io->chebrip = (NULL == cheb_rip ? 3. : *cheb_rip);
		break;
	case iir_butterworth: 
		io->options |= mkfilter_opt_bu; 
		break;
	default:
		throw std::invalid_argument("invalid filter type specification");
	}

	const unsigned char_mask = 0x01f0;
	switch (type & char_mask) {
	case iir_lowpass: 
		io->options |= mkfilter_opt_lp | mkfilter_opt_a;
		io->raw_alpha1 = *fc;
		break;
	case iir_highpass:
		io->options |= mkfilter_opt_hp | mkfilter_opt_a;
		io->raw_alpha1 = *fc;
		break;
	case iir_bandpass:
		io->options |= mkfilter_opt_bp | mkfilter_opt_a;
		io->raw_alpha1 = fc[0];
		io->raw_alpha2 = fc[1];
		sz = 2 * order;
		break;
	case iir_bandstop:
		io->options |= mkfilter_opt_bs | mkfilter_opt_a;
		io->raw_alpha1 = fc[0];
		io->raw_alpha2 = fc[1];
		sz = 2 * order;
		break;
	case iir_allpass:
		io->options |= mkfilter_opt_ap | mkfilter_opt_a;
		io->raw_alpha1 = *fc;
		break;
	default:
		throw std::invalid_argument("missing filter type specification");
	}

	if (NULL != zero_freq) {
		io->options |= mkfilter_opt_Z;
		io->raw_alphaz = *zero_freq;
	}
	if (0 != pole_mask) {
		io->options |= mkfilter_opt_p;
		io->polemask = pole_mask;
	}
	if (type & iir_prewrap)
		io->options |= mkfilter_opt_w;
	if (type & iir_matched_z)
		io->options |= mkfilter_opt_z;
	io->order = static_cast<int>(order);
	io->options |= mkfilter_opt_o;

	mkfilter::design(*io);

	// inverse coeffs order - mkfilter produces vectors in the format for its own internal filter implementation which uses inversed vectors (and negative denominator)
	for (size_t i = 0; i < sz + 1; ++i) {
		b[i] = io->xcoeffs_r[sz - i];
		a[i] = -io->ycoeffs_r[sz - i];
	}
}
