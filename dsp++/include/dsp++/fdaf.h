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
		calloc_.deallocate(cbuf_, 6*N_);
		ralloc_.deallocate(rbuf_, 6*N_);
	}

	fdaf_overlap_save(size_t block_length, value_type mu, value_type leakage = value_type(1))
	 :	N_(block_length)
	 ,	rbuf_(ralloc_.allocate(6*N_))
	 ,	cbuf_(calloc_.allocate(6*N_))
	 ,	dft_(2*N_, rbuf_, cbuf_)
	 ,	idft_(2*N_, cbuf_, rbuf_)
	 ,	x_(rbuf_)
	 ,	e_(x_ + 2*N_)
	 ,	y_(e_ + 2*N_)
	 ,	d_(e_ + N_)
	 ,	v_(e_)
	 ,	X_(cbuf_)
	 ,	E_(X_ + 2*N_)
	 ,	W_(E_ + 2*N_)
	 ,	mu_(mu)
	 ,	lambda_(leakage)
	{
		std::fill_n(rbuf_, 6*N_, value_type());
		std::fill_n(cbuf_, 6*N_, complex_type());
	}

	iterator x_begin() {return x_ + N_;}
	const_iterator x_begin() const {return x_ + N_;}
	iterator x_end() {return x_ + 2*N_;}
	const_iterator x_end() const {return x_ + 2*N_;}
	std::pair<iterator, iterator> x() {return std::make_pair(x_begin(), x_end());}
	std::pair<const_iterator, const_iterator> x() const {return std::make_pair(x_begin(), x_end());}

	//iterator y_begin() {return y_;}
	const_iterator y_begin() const {return y_;}
	//iterator y_end() {return y_ + N_;}
	const_iterator y_end() const {return y_ + N_;}
	std::pair<const_iterator, const_iterator> y() const {return std::make_pair(y_begin(), y_end());}

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
	std::pair<iterator, iterator> W() {return std::make_pair(W_begin(), W_end());}
	std::pair<const_iterator, const_iterator> W() const {return std::make_pair(W_begin(), W_end());}

	//! @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type step_size() const {return mu_;}
	//! @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_step_size(const value_type mu) {mu_ = mu;}
	//! @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type mu() const {return mu_;}
	//! @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_mu(const value_type mu) {mu_ = mu;}

	value_type leakage() const {return lambda_;}
	void set_leakage(const value_type lambda) {lambda_ = lambda;}

	void operator()() {
		dft_(x_, X_);													// calculate FFT of x() and put it into X
		std::transform(X_, X_ + 2*N_, W_, E_, std::multiplies<complex_type>());			// y = IFFT{X * W}, using E_ as temporary variable
		idft_(E_, y_);						
		std::transform(y_ + N_, y_ + 2*N_, y_ + N_, std::bind2nd(std::divides<value_type>(), 2*N_)); // scale IFFT and store output into 2nd half of y
		std::transform(d_, d_ + N_, y_ + N_, d_, std::minus<value_type>());	// subtract y() from d() to form 2nd half of e()
		std::transform(e_ + N_, e_ + 2*N_, e_ + N_, std::bind2nd(std::multiplies<value_type>(), mu_));	// apply step_size e() = e() * mu
		std::fill_n(e_, N_, value_type());								// zero first half of e()
		dft_(e_, E_);													// put FFT of zero-prepended e() into E
		complex_iterator E = E_, X = X_;
		for (size_t i = 0; i < 2*N_; ++i, ++E, ++X)						// multiply F{e()} with conjugate copy of F{x()} (no functor for 2 operations at once?)
			(*E) *= std::conj(*X);
		idft_(E_, v_);													// perform IFFT on multiplication result, calculate gradient constraint
		std::transform(v_, v_ + N_, v_, std::bind2nd(std::divides<value_type>(), 2*N_));		// scale IFFT output, but only for the useful half
		std::fill_n(v_ + N_, N_, value_type());							// discard and zero-fill 2nd half of v()
		dft_(v_, E_);													// FFT v() back to DFT domain
		complex_iterator W = W_; E = E_, X = X_;
		for (size_t i = 0; i < 2*N_; ++i, ++W, ++E, ++X) {				// update W with forgetting-factor-multiplied error transform
			*W *= lambda_;
			*W += (*E);													// W(k + 1) = W(k) + 2uF{v(n)}
		}
		std::copy(x_ + N_, x_ + 2*N_, x_);								// store current x input in the 1st half of x as "previous" frame
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
	value_type* const e_;	//!< (2N) error input vector, e[0..N) will be filled with 0s, e[N..2N) is d() - y()
	value_type* const y_;	//!< (2N) output vector 
	value_type* const d_;	//!< (N) desired response input vector, this is convenience as it will point to the 2nd half of e_
	value_type* const v_;	//!< (2N) gradient constraint calculation vector, this is convenience only as e_ will be reused
	complex_type* const X_;	//!< (2N) FFT(x()) output
	complex_type* const E_;	//!< (2N) FFT(e()) output
	complex_type* const W_; //!< (2N) adaptive weights transform

	value_type mu_;				//!< step size mu
	value_type lambda_;			//!< leakage factor 

};

}

#endif /* DSP_FDAF_H_INCLUDED */
