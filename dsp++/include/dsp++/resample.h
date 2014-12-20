/**
 * @file dsp++/resample.h
 * @brief Sample rate conversion
 */
#ifndef DSP_RESAMPLE_H_INCLUDED
#define DSP_RESAMPLE_H_INCLUDED
#pragma once

#include <dsp++/export.h>
#include <dsp++/filter.h>
#include <dsp++/stride_iterator.h>
#include <vector>

namespace dsp {

//! @brief Design antialiasing lowpass FIR filter for interpolator/decimator with given factor.
//! Uses Parks-McClellan algorithm internally.
//! @see dsp::fir::pm::design()
//! @param[in] order filter order, filter will have (order + 1) coefficients
//! @param[out] coeffs vector with space for (order + 1) FIR filter coefficients returned upon completion
//! @param[in] factor interpolator/decimator (integer) factor
//! @param[in] transition_width width of transition region as a percentage of fullband
DSPXX_API void antialiasing_filter_design(size_t order, double* coeffs, size_t factor, double transition_width);

template<class Sample>
class interpolator_base {
public:
	typedef typename dsp::trivial_array<Sample>::const_iterator const_iterator;

	//! @return interpolation factor and the length of output sequence
	size_t factor() const {return M_;}
	//! @return interpolation (lowpass) filter order
	size_t order() const {return P_;}

protected:
	interpolator_base(size_t M, size_t P, size_t block_size)
	 :	M_(M)
	 ,	P_(P)
	 ,	buf_(M_ * block_size)
	 ,	output(buf_.begin(), buf_.end())
	{
	}

	size_t const M_;			//!< interpolation factor and number of polyphase filters
	size_t const P_;			//!< filter order
	dsp::trivial_array<Sample> buf_;	//!< output buffer (M_ * block_size)
public:

	ioport_ro<const_iterator> output;
};

//! @brief Integer-factor interpolator using polyphase FIR structure, operating on a single input sample each pass.
template<class Sample>
class interpolator: public interpolator_base<Sample> {
	typedef dsp::filter<Sample> filter_type;
	typedef interpolator_base<Sample> base;
public:

	//! @brief Initialize iterpolator for given factor M, using lowpass FIR of order P and given transition region width as an antialiasing filter.
	//! @param[in] M iterpolation factor
	//! @param[in] P antialiasing filter order
	//! @param[in] transition_width width of antialiasing filter transition region as a percentage of fullband
	interpolator(size_t M, size_t P, double transition_width = 0.2)
	 :	base(M, P, 1)
	{
		init_filters(transition_width);
	}


	~interpolator() {
		cleanup_filters();
	}

	//! @brief Pass next sample to the interpolator, outputting M (factor) resulting samples in internal buffer [begin(), end()).
	void operator()(Sample x) {
		for (size_t i = 0; i < base::M_; ++i)
			base::buf_[i] = (*flt_[i])(x);
	}

private:
	std::vector<filter_type*> flt_;		//!< polyphase filter sections

	void cleanup_filters() {
		for (size_t i = 0; i < flt_.size(); ++i)
			delete flt_[i];
		flt_.clear();
	}

	void init_filters(double transition);
};

template<class Sample>
void interpolator<Sample>::init_filters(double transition) 
{
	size_t p = (base::P_ + base::M_ - 1) / base::M_;		// length of single phase section
	size_t bn = p * base::M_;								// total number of coefficients
	std::vector<double> buf(bn + p);						// space for bn coefficients as well as p coeffs for a single phase
	antialiasing_filter_design(bn - 1, &buf[0], base::M_, transition);

	flt_.reserve(base::M_);
	try {
		for (size_t i = 0; i < base::M_; ++i) {				// deinterleave coefficients for each phase
			for (size_t j = 0; j < p; ++j) 
				buf[bn + j] = base::M_ * buf[i + j * base::M_];			
			flt_.push_back(new filter_type(&buf[bn], p));
		}
	}
	catch (...) {
		cleanup_filters();
		throw;
	}
}

//! @brief Integer-factor interpolator using polyphase FIR structure, operating on a block of input samples each pass.
template<class Sample>
class block_interpolator: public interpolator_base<Sample> {
	typedef dsp::block_filter<Sample> filter_type;
	typedef interpolator_base<Sample> base;
public:
	typedef typename dsp::trivial_array<Sample>::iterator iterator;
	typedef typename dsp::trivial_array<Sample>::const_iterator const_iterator;

	//! @brief Initialize iterpolator for given factor M, using lowpass FIR of order P and given transition region width as an antialiasing filter.
	//! @param[in] L input block length (output block will have L*M samples)
	//! @param[in] M iterpolation factor
	//! @param[in] P antialiasing filter order
	//! @param[in] transition_width width of antialiasing filter transition region as a percentage of fullband
	block_interpolator(size_t L, size_t M, size_t P, double transition_width = 0.2)
	 :	base(M, P, L)
	 ,	L_(L)
	 ,	input(base::buf_.begin(), L_)
	{
		init_filters(transition_width);
	}


	~block_interpolator() {
		cleanup_filters();
	}

	//! @brief Perform interpolation using L (input_length()) samples of input sequence ([in, in + L)) as an input, placing output sequence in internal buffer [begin(), end()).
	//! @param[in] in start of L-length input sequence (following iterator abstraction).
	template<class Iterator>
	void operator()(Iterator in) {
		for (size_t i = 0; i < base::M_; ++i) {
			dsp::copy_n(in, L_, flt_[i]->input.begin());
			(*flt_[i])();
		}
		size_t total = output_length();
		for (size_t i = 0; i < base::M_; ++i)
			std::copy(flt_[i]->output.begin(), flt_[i]->output.end(), dsp::make_stride(base::buf_.begin(), base::M_, i));
	}

	//! @brief Perform interpolation inplace using first L (input_length()) samples of [begin(), end()) sequence as an input.
	void operator()() {
		operator()(input.begin());
	}

	//! @return length of input sequence
	size_t input_length() const {return L_;}
	//! @return length of output sequence
	size_t output_length() const {return L_ * base::M_;}

private:
	size_t const L_;
	std::vector<filter_type*> flt_;		//!< polyphase filter sections

	void cleanup_filters() {
		for (size_t i = 0; i < flt_.size(); ++i)
			delete flt_[i];
		flt_.clear();
	}

	void init_filters(double transition);

public:

	ioport_rw<const_iterator, iterator> input;
};

template<class Sample>
void block_interpolator<Sample>::init_filters(double transition) 
{
	size_t p = (base::P_ + base::M_ - 1) / base::M_;		// length of single phase section
	size_t bn = p * base::M_;								// total number of coefficients
	std::vector<double> buf(bn + p);						// space for bn coefficients as well as p coeffs for a single phase
	antialiasing_filter_design(bn - 1, &buf[0], base::M_, transition);

	flt_.reserve(base::M_);
	try {
		for (size_t i = 0; i < base::M_; ++i) {				// deinterleave coefficients for each phase
			for (size_t j = 0; j < p; ++j) 
				buf[bn + j] = base::M_ * buf[i + j * base::M_];			
			flt_.push_back(new filter_type(L_, &buf[bn], p));
		}
	}
	catch (...) {
		cleanup_filters();
		throw;
	}
}


}

#endif // DSP_RESAMPLE_H_INCLUDED
