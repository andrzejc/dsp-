#include <dsp++/resample.h>
#include <dsp++/filter_design.h>

void dsp::antialiasing_filter_design(size_t order, double* coeffs, size_t factor, double transition)
{
	double freqs[4] = {0, (1. - transition) / (2 * factor), (1. + transition) / (2 * factor), .5};
	double amps[4] = {1., 1., 0., 0.};
	double wgt[2] = {1., 1.};
	fir::pm::design(static_cast<unsigned>(order), 2, freqs, amps, wgt, coeffs, fir::pm::type::type_I_II, 24);
}

