/**
 * @file dsp++/resample.h
 * @brief Sample rate conversion
 */
#ifndef DSP_RESAMPLE_H_INCLUDED
#define DSP_RESAMPLE_H_INCLUDED
#pragma once

#include <dsp++/export.h>
#include <dsp++/filter_design.h>
#include <dsp++/filter.h>

#include <boost/foreach.hpp>

namespace dsp {

template<class Sample>
class interpolator {
	typedef dsp::filter<Sample> filter_type;
public:

	interpolator(size_t M, size_t P, double transition_width = 0.05)
	 :	M_(M)
	 ,	P_(P)
	 ,	buf_(M_)
	{
		init_filters(transition_width);
	}


	~interpolator() {
		cleanup_filters();
	}

	void operator()(Sample x) {
		for (size_t i = 0; i < M_; ++i) 
			buf_[i] = (*flt_[i])(x);
	}

	//! @return starting iterator of the output (M-length) sequence
	const Sample* begin() const {return buf_.begin();}
	//! @return end iterator of the output (M-length) sequence
	const Sample* end() const {return buf_.end();}
	//! @return interpolation factor and the length of output sequence
	size_t factor() const {return M_;}
	//! @return interpolation (lowpass) filter order
	size_t order() const {return P_;}

private:
	size_t const M_;			//!< interpolation factor and number of polyphase filters
	size_t const P_;			//!< filter order
	dsp::trivial_array<Sample> buf_;	//!< output buffer (M_)
	std::vector<filter_type*> flt_;		//!< polyphase filter sections

	void cleanup_filters() {
		BOOST_FOREACH(filter_type* f, flt_)
			delete f;
		flt_.clear();
	}

	void init_filters(double transition);
};

template<class Sample>
void interpolator<Sample>::init_filters(double transition) 
{
	size_t p = (P_ + M_ - 1) / M_;		// length of single phase section
	size_t bn = p * M_;					// total number of coefficients
	std::vector<double> buf(bn + p);	
	double freqs[4] = {0, (1. - transition) / (2 * M_), (1. + transition) / (2 * M_), .5};
	double amps[4] = {1., 1., 0., 0.};
	double wgt[2] = {1., 1.};
	firpm(bn - 1, &buf[0], 2, freqs, amps, wgt);		// design filter with remez algorithm

	flt_.reserve(M_);
	try {
		for (size_t i = 0; i < M_; ++i) {
			for (size_t j = 0; j < p; ++j) 
				buf[bn + j] = buf[i + j * M_];			// prepare set of coefficients for single phase section
			flt_.push_back(new filter_type(&buf[bn], p));
		}
	}
	catch (...) {
		cleanup_filters();
		throw;
	}
}

}

#endif // DSP_RESAMPLE_H_INCLUDED