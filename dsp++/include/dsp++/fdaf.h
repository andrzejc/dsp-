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

template<class Real, template<class, class> class DFT = dsp::fft>
class fdaf_overlap_save: private noncopyable
{
public:
	typedef Real value_type;
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
		ralloc_.deallocate(rbuf_, 4*N_);
	}

	fdaf_overlap_save(size_t block_length, Real mu)
	 :	N_(block_length)
	 ,	rbuf_(ralloc_.allocate(4*N_))
	 ,	cbuf_(calloc_.allocate(6*N_))
	 ,	dft_(2*N_, rbuf_, cbuf_)
	 ,	idft_(2*N_, cbuf_, rbuf_)
	 ,	x_(rbuf_)
	 ,	e_(x_ + 2*N_)
	 ,	y_(e_)
	 ,	d_(e_ + N_)
	 ,	v_(e_)
	 ,	X_(cbuf_)
	 ,	E_(X_ + 2*N_)
	 ,	W_(E_ + 2*N_)
	 ,	mu_(mu)
	{
		std::fill_n(rbuf_, 4*N_, real_value());
		std::fill_n(cbuf_, 6*N_, complex_value());
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

	value_type forgetting_factor() const {return mu_;}
	value_type mu() const {return mu_;}
	void set_mu(value_type mu) {mu_ = mu;}
	void set_forgetting_factor(value_type mu) {mu_ = mu;}

	void operator()() {
		dft_(x_, X_);													// calculate FFT of x() and put it into X
		std::transform(d_, d_ + N_, y_, d_, std::minus<value_type>());	// subtract y() from d() to form 2nd half of e()
		std::fill_n(e_, N_, value_type());								// zero first half of e()
		dft_(e_, E_);													// put FFT of zero-prepended e() into E
		for (size_t i = 0; i < 2*N_; ++i)								// multiply F{e()} with conjugate copy of F{x()} (no functor for 2 operations at once?)
			E_[i] *= std::conj(X_[i]);
		idft_(E_, v_);													// perform IFFT on multiplication result, calculate gradient constraint
		std::fill_n(v_ + N_, N_, value_type());							// discard and zero-fill 2nd half of v()
		dft_(v_, E_);													// FFT v() back to DFT domain
		for (size_t i = 0; i < 2*N_; ++i) {								// update W with forgetting-factor-multiplied error transform
			W[i] += E[i] * 2* mu_;										// W(k + 1) = W(k) + 2uF{v(n)}
			X[i] *= W[i];												// calculate Y(k) = X(k) * W(k) in the same loop
		}
		idft_(X_, e_);													// calculate y(n) = IFFT{Y(k)}
		std::copy(e_ + N_, e_ + 2*N_, y_);								// store only 2nd half of the y(), copy it into y output vector
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
	value_type* const y_;	//!< (N) output vector, this is convenience only as it will point to the first half of e_ 
	value_type* const d_;	//!< (N) desired response input vector, this is convenience as it will point to the 2nd half of e_
	value_type* const v_;	//!< (2N) gradient constraint calculation vector, this is convenience only as e_ will be reused
	complex_type* const X_;	//!< (2N) FFT(x()) output
	complex_type* const E_;	//!< (2N) FFT(e()) output
	complex_type* const W_; //!< (2N) adaptive weights transform

	Real mu_;				//!< forgetting factor mu
};

}

#endif /* DSP_FDAF_H_INCLUDED */
