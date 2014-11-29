/*!
 * @file dsp++/fdaf.h
 * @brief Frequency-Domain Adaptive Filters.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_FDAF_H_INCLUDED
#define DSP_FDAF_H_INCLUDED
#pragma once

#include <dsp++/config.h>
#include <dsp++/fft.h>
#include <dsp++/pow2.h>
#include <dsp++/algorithm.h>
#include <dsp++/noncopyable.h>

#include <algorithm>
#include <functional>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif // !DSP_BOOST_CONCEPT_CHECKS_DISABLED

namespace dsp {

template<class Sample, template<class, class> class DFT = dsp::fft>
class fdaf_overlap_save: private noncopyable
{
public:
	typedef Sample value_type;
	typedef std::complex<value_type> complex_type;
	typedef DFT<value_type, complex_type> transform_type;
	typedef DFT<complex_type, value_type> inverse_transform_type;
	typedef typename transform_type::input_allocator real_allocator;
	typedef typename transform_type::output_allocator complex_allocator;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;
	typedef complex_type* complex_iterator;
	typedef const complex_type* const_complex_iterator;


	~fdaf_overlap_save() {
		calloc_.deallocate(cbuf_, 8*N_);
		ralloc_.deallocate(rbuf_, 8*N_);
	}

	fdaf_overlap_save(size_t block_length, value_type mu, value_type leakage = value_type(1), value_type init_pow = value_type(1), value_type avg_fact = value_type(0.9), value_type offset = value_type())
	 :	N_(block_length)
	 ,	rbuf_(ralloc_.allocate(8*N_))
	 ,	cbuf_(calloc_.allocate(8*N_))
	 ,	dft_(2*N_, rbuf_, cbuf_)
	 ,	idft_(2*N_, cbuf_, rbuf_)
	 ,	x_(rbuf_)
	 ,	y_(x_ + 2*N_)
	 ,	w_(y_ + 2*N_)
	 ,	e_(w_ + 2*N_)
	 ,	d_(e_ + N_)
	 ,	X_(cbuf_)
	 ,	E_(X_ + 2*N_)
	 ,	W_(E_ + 2*N_)
	 ,	norm_(W_ + 2*N_)
	 ,	mu_(mu)
	 ,	lambda_(leakage)
	 ,	offset_(offset)
	 ,	beta_(avg_fact)
	{
		std::fill_n(rbuf_, 8*N_, value_type());
		std::fill_n(cbuf_, 6*N_, complex_type());
		std::fill_n(norm_, 2*N_, complex_type(init_pow));
	}

	size_t transform_size() const {return 2*N_;}
	size_t block_length() const {return N_;}

	iterator x_begin() {return x_ + N_;}
	const_iterator x_begin() const {return x_ + N_;}
	iterator x_end() {return x_ + 2*N_;}
	const_iterator x_end() const {return x_ + 2*N_;}
	std::pair<iterator, iterator> x() {return std::make_pair(x_begin(), x_end());}
	std::pair<const_iterator, const_iterator> x() const {return std::make_pair(x_begin(), x_end());}

	const_iterator y_begin() const {return y_ + N_;}
	const_iterator y_end() const {return y_ + 2*N_;}
	std::pair<const_iterator, const_iterator> y() const {return std::make_pair(y_begin(), y_end());}

	const_iterator e_begin() const {return e_;}
	const_iterator e_end() const {return e_ + N_;}
	std::pair<const_iterator, const_iterator> e() const {return std::make_pair(e_begin(), e_end());}

	iterator d_begin() {return d_;}
	const_iterator d_begin() const {return d_;}
	iterator d_end() {return d_ + N_;}
	const_iterator d_end() const {return d_ + N_;}
	std::pair<iterator, iterator> d() {return std::make_pair(d_begin(), d_end());}
	std::pair<const_iterator, const_iterator> d() const {return std::make_pair(d_begin(), d_end());}

	complex_iterator W_begin() {return W_;}
	const_complex_iterator W_begin() const {return W_;}
	complex_iterator W_end() {return W_ + 2*N_;}
	const_complex_iterator W_end() const {return W_+2*N_;}
	complex_iterator W_half_end() {return W_ + N_ + 1;}
	const_complex_iterator W_half_end() const {return W_ + N_ + 1;}
	std::pair<iterator, iterator> W() {return std::make_pair(W_begin(), W_end());}
	std::pair<const_iterator, const_iterator> W() const {return std::make_pair(W_begin(), W_end());}
	std::pair<iterator, iterator> W_half() {return std::make_pair(W_begin(), W_half_end());}
	std::pair<const_iterator, const_iterator> W_half() const {return std::make_pair(W_begin(), W_half_end());}
	size_t transform_half_size() const {return N_ + 1;}

	//! @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type step_size() const {return mu_;}
	//! @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type mu() const {return mu_;}
	//! @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_step_size(const value_type mu) {mu_ = mu;}
	//! @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_mu(const value_type mu) {mu_ = mu;}

	value_type leakage() const {return lambda_;}
	void set_leakage(const value_type lambda) {lambda_ = lambda;}

	value_type offset() const {return offset_;}
	void set_offset(value_type offset) {offset_ = offset;}

	value_type averaging_factor() const {return beta_;}
	void set_averaging_factor(value_type beta) {beta_ = beta;}

	//! @brief Store current x input vector as previous frame.
	//! This is useful in the cases when adaptation needs to be paused while the data is running.
	void tick_input() {
		std::copy(x_ + N_, x_ + 2*N_, x_);								// store current x input in the 1st half of x as "previous" frame
	}

	//! @brief Process single buffer of data through adaptive algoritm, taking [x_begin(), x_end()) as x input and [d_begin(), d_end()) as the expected filter output.
	//! The results are stored in [y_begin(), y_end()) (filter output) and [e_begin(), e_end()) (output error w/ regard to d). 
	//! The FFT of filter weights (coefficients) may be read from [W_begin(), W_end()).
	void operator()() {
		dft_(x_, X_);															// X = FFT{x()}
		std::transform(X_, X_ + 2*N_, W_, E_, std::multiplies<complex_type>());	// y() = IFFT{X * W}, using E_ as temporary variable
		idft_(E_, y_);						
		std::transform(y_ + N_, y_ + 2*N_, y_ + N_, std::bind2nd(std::divides<value_type>(), 2*N_)); // scale IFFT (only applied to 2nd half of y_, 1st one is irrelevant)
		std::transform(d_, d_ + N_, y_ + N_, e_, std::minus<value_type>());		// e() = d() - y(), e_ is now ready
		std::transform(e_, e_ + N_, w_ + N_, std::bind2nd(std::multiplies<value_type>(), mu_));	// apply step_size e() = e() * mu, put result in 2nd half of w_
		std::fill_n(w_, N_, value_type());								// zero first half of w_
		dft_(w_, E_);													// put FFT of zero-prepended e() into E

		value_type ombet = value_type(1) - beta_;
		complex_iterator E = E_, N = norm_, X = X_;
		for (size_t i = 0; i < 2*N_; ++i, ++E, ++N, ++X) {			
			complex_type cX = std::conj(*X);
			(*N *= beta_) += ombet * std::real(*X * cX);	// update signal power 
			(*E) *= (cX / (*N + offset_));					// multiply E with conjugate of X and normalize
		}
		idft_(E_, w_);													// perform IFFT on multiplication result, calculate gradient constraint
		std::transform(w_, w_ + N_, w_, std::bind2nd(std::divides<value_type>(), 2*N_));	// scale IFFT output, but only for the useful half
		std::fill_n(w_ + N_, N_, value_type());							// discard and zero-fill 2nd half of w_
		dft_(w_, E_);													// FFT w_ back to DFT domain
		complex_iterator W = W_; E = E_;
		for (size_t i = 0; i < 2*N_; ++i, ++W, ++E) 					// update W with forgetting-factor-multiplied error transform
			(*W *= lambda_) += *E;

		tick_input();
	}


private:

	real_allocator ralloc_;
	complex_allocator calloc_;

	const size_t N_;		//!< frame & impulse response length
	value_type* rbuf_;		//!< real-valued buffer allocated through ralloc_
	complex_type* cbuf_;	//!< complex-valued buffer allocated through calloc_
	transform_type dft_;			//!< DFT functor
	inverse_transform_type idft_;	//!< IDFT functor

	value_type* const x_;	//!< (2N) input vector, x[0..N) is old (saved) input frame frame, x[N..2N) is the new input frame upon input 
	value_type* const y_;	//!< (2N) output vector 
	value_type* const w_;	//!< (2N) real-valued work area
	value_type* const e_;	//!< (N) error output vector, e() = d() - y()
	value_type* const d_;	//!< (N) desired response input vector
	complex_type* const X_;	//!< (2N) FFT(x()) output
	complex_type* const E_;	//!< (2N) FFT(e()) output
	complex_type* const W_; //!< (2N) adaptive weights transform
	complex_type* const norm_;	//!< (2N) signal power for normalization

	value_type mu_;				//!< step size mu
	value_type lambda_;			//!< leakage factor 
	value_type offset_;			//!< normalization offset to avoid divide by zero
	value_type beta_;			//!< averaging factor for exponential averaning of input power
};

}

#endif /* DSP_FDAF_H_INCLUDED */
