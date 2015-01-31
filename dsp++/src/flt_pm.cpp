#include <dsp++/flt/fir_design.h>
#include <stdexcept>
#include "remez/remez.h"

bool dsp::fir::pm::design(unsigned order, unsigned band_count, double freqs[], const double amps[],
	const double weights[], double h[], enum_class_ref(type) type, unsigned grid_density, unsigned max_iterations)
{
	remez_filter_type ft;
	switch (type) {
	case pm::type::differentiator: ft = REMEZ_FILTER_DIFFERENTIATOR; break;
	case pm::type::hilbert: ft = REMEZ_FILTER_HILBERT; break;
	default: ft = REMEZ_FILTER_BANDPASS; break;
	}
	double lf = 0.;
	for (unsigned i = 0; i < 2 * band_count; ++i)
	{
		double f = freqs[i];
		if (f < 0. || f > 0.5)
			throw std::domain_error("dsp::fir::pm::design(): freqs outside [0, 0.5]");
		if (f < lf)
			throw std::invalid_argument("dsp::fir::pm::design(): freqs non-monotonic");
		lf = f;
	}
	for (unsigned i = 0; i < band_count; ++i)
		if (weights[i] <= 0.)
			throw std::domain_error("dsp::fir::pm::design(): weights <= 0");

	int res = remez(h, static_cast<int>(order + 1), static_cast<int>(band_count), freqs, amps, weights, ft, static_cast<int>(grid_density), static_cast<int>(max_iterations));
	if (REMEZ_FAILED(res))
		throw std::bad_alloc();

	return (REMEZ_NOERR == res);
}


