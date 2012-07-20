/*!
 * @file filter_design.cpp
 * @brief Implementation of filter design algorithms.
 */
#include <dsp++/filter_design.h>
#include "remez/remez.h"

#include <stdexcept>
#include <cmath>

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

	double w0 = M_PI * 2. * norm_freq; 	// 2 * pi * f0 / Fs
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
		alpha = sw0 / 2 * (*q);
	else if (NULL != bw)
		alpha = sw0 * std::sinh(M_LN2 / 2 * (*bw) * w0/sw0);
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
		a[0] = (Ap1 + Am1c + sqAa2); b[1] = -2 * (Am1 + Ap1c); b[2] = Ap1 + Am1c - sqAa2;
		break;
	case biquad_type_high_shelf_eq:
		b[0] = A * (Ap1 + Am1c + sqAa2); b[1] = -2 * A * (Am1 + Ap1c); b[2] = A * (Ap1 + Am1c - sqAa2);
		a[0] = (Ap1 - Am1c + sqAa2); b[1] = 2 * (Am1 - Ap1c); b[2] = Ap1 - Am1c - sqAa2;
		break;
	}
}
