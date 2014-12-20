#include <dsp++/flt/biquad_design.h>
#include <dsp++/const.h>
#include <stdexcept>

void dsp::biquad::design(double b[], double a[], biquad::type::spec type, double norm_freq, const double* gain_db, const double* q, const double* bw, const double* s)
{
	if (norm_freq < 0. || norm_freq > 0.5)
		throw std::domain_error("dsp::biquad::design(): norm_freq outside [0, 0.5]");

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
			throw std::invalid_argument("dsp::biquad::design(): gain_db not specified");
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
		throw std::invalid_argument("dsp::biquad::design(): q, bw, s not specified");

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
