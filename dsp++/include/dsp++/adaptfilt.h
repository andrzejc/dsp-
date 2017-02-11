/// @file dsp++/adaptfilt.h
/// @brief Implementation of time-domain adaptive filtering.
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_ADAPTFILT_H_INCLUDED
#define DSP_ADAPTFILT_H_INCLUDED
#pragma once

#include <dsp++/config.h>
#include <dsp++/utility.h>
#include <dsp++/buffer_traits.h>
#include <dsp++/trivial_array.h>
#include <dsp++/simd.h>
#include <dsp++/complex.h>
#include <dsp++/ioport.h>
#include <dsp++/concept_checks.h>

#include <algorithm>


namespace dsp {

/// @brief Perform single step of adaptive Least Mean Squares (LMS) filter algorithm.
/// @see https://en.wikipedia.org/wiki/Least_mean_squares_filter
/// @param[in] d sample of the observed signal \f$d(n) = y(n) + v(n)\f$, \f$v(n)\f$ being the additive disturbance and
/// 	\f$y(n) = \mathbf{h}^H(n)\cdot \mathbf{x}(n)\f$.
/// @param[in] x excitation vector \f$\mathbf{x}(n) = \left[x(n), x(n-1),\cdots,x(n-P+1)\right]^T\f$ of length P, x[0] is the most recent sample \f$x(n)\f$.
/// @param[in,out] h estimated filter response \f$\hat{\mathbf{h}}(n)\f$ of length P, updated with this iteration estimate upon return:
///		\f$\hat{\mathbf{h}}(n+1) = \lambda\hat{\mathbf{h}}(n) + \mu\,e^{\ast}(n)\mathbf{x}(n)\f$.
/// @param[in] P order of the adaptive filter and number of elements in vectors x and h.
/// @param[in] mu convergence step size  \f$\mu\f$.
/// @param[in] lambda leakage factor (1 - no leakage).
/// @return estimation error \f$e(n) = d(n) - \hat{y}(n) = d(n) - \hat{\mathbf{h}}^H(n) \cdot \mathbf{x}(n)\f$.
template<class Sample>
Sample filter_sample_adapt_lms(const Sample d, const Sample* x, Sample* h, const size_t P, const Sample mu, const Sample lambda = Sample(1))
{
	Sample e = d;
	for (size_t i = 0; i < P; ++i)
		e -= conj(h[i])  * x[i];
	Sample ec = conj(e);
	for (size_t i = 0; i < P; ++i) {
		h[i] *= lambda;
		h[i] += mu * ec * x[i];
	}
	return e;
}

/// @brief Perform single step of adaptive Normalised LMS (NLMS) filter algorithm.
/// @see https://en.wikipedia.org/wiki/Least_mean_squares_filter
/// @param[in] d sample of the observed signal \f$d(n) = y(n) + v(n)\f$, \f$v(n)\f$ being the additive disturbance and
///		\f$y(n) = \mathbf{h}^H(n)\cdot \mathbf{x}(n)\f$.
/// @param[in] x excitation vector \f$\mathbf{x}(n) = \left[x(n), x(n-1),\cdots,x(n-P+1)\right]^T\f$ of length P,
///		x[0] is the most recent sample \f$x(n)\f$.
/// @param[in,out] h estimated filter response \f$\hat{\mathbf{h}}(n)\f$ of length P, updated with this iteration estimate upon return:
///		\f$\hat{\mathbf{h}}(n+1) = \lambda\hat{\mathbf{h}}(n) + \frac{\mu\,e^{\ast}(n)\mathbf{x}(n)}{\mathbf{x}^H(n)\mathbf{x}(n) + \gamma}\f$.
/// @param[in] P order of the adaptive filter and number of elements in vectors x and h.
/// @param[in] mu convergence step size  \f$\mu\f$.
/// @param[in] gamma denominator offset \f$\gamma\f$ safeguarding agains divide-by-zero in case of 0 signal.
/// @param[in] lambda leakage factor \f$\lambda\f$ (1 - no leakage).
/// @tparam Sample real or complex type of processed values
/// @return estimation error \f$e(n) = d(n) - \hat{y}(n) = d(n) - \hat{\mathbf{h}}^H(n) \cdot \mathbf{x}(n)\f$.
template<class Sample>
Sample filter_sample_adapt_nlms(const Sample d, const Sample* x, Sample* h, const size_t P, const Sample mu, const Sample gamma = Sample(), const Sample lambda = Sample(1))
{
	Sample e = d;
	Sample p = Sample();
	for (size_t i = 0; i < P; ++i) {
		e -= conj(h[i])  * x[i];
		p += conj(x[i]) * x[i];
	}
	Sample ec = conj(e);
	for (size_t i = 0; i < P; ++i) {
		h[i] *= lambda;
		h[i] += mu * ec * x[i] / (p + gamma);
	}
	return e;
}

/// @brief Common base class for implementing a family of LMS adaptive filter functors.
/// @tparam Sample real or comples type of the processed signal samples.
template<class Sample, class BufferTraits = dsp::buffer_traits<Sample> >
class lms_filter_base {
public:
	typedef Sample value_type;				///< Type of sample (value) this filter operates with.
	typedef Sample* iterator;				///< Type of iterator used to access the estimated filter response.
	typedef const Sample* const_iterator;	///< Type of const iterator used to access the estimated filter response.

	/// @return Order of the implemented LMS adaptive filter P.
	size_t order() const {return P_;}
	/// @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type step_size() const {return mu_;}
	/// @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_step_size(const value_type mu) {mu_ = mu;}
	/// @return Step size \f$\mu\f$ of the LMS algorithm.
	value_type mu() const {return mu_;}
	/// @brief Modify step size \f$\mu\f$ of the LMS algorithm. @param[in] mu new step size used during subsequent iterations.
	void set_mu(const value_type mu) {mu_ = mu;}
	/// @return Filter leakage (1 - no leakage at all).
	// TODO explain what is leakage...
	value_type leakage() const {return lambda_;}
	/// @brief Modify leakage \f$\lambda\f$ of LMS algorithm. @param[in] lambda new leakage value used during subsequent iterations (1 - no leakage at all).
	void set_leakage(const value_type lambda) {lambda_ = lambda;}

protected:
	lms_filter_base(const size_t P, const Sample mu, const Sample lambda = Sample(1), const Sample* initial_h = NULL)
	 :	P_(P)
	 ,	P_pad_(BufferTraits::aligned_count(P_))
	 ,	buffer_(2 * P_pad_)
	 ,	x_(buffer_.get())
	 ,	h_(x_ + P_pad_)
	 ,	mu_(mu)
	 ,	lambda_(lambda)
	 ,	h(h_, P_)
	{
		std::fill_n(x_, P_pad_, Sample());
		if (NULL != initial_h)
			std::copy_n(initial_h, P, h_);
		else
			std::fill_n(h_, P_pad_, Sample());
	}

protected:
	const size_t P_;		///< filter order and length of x_ and h_ vectors
	const size_t P_pad_;	///< length of allocated space of x_ and h_ vectors
	trivial_array<Sample, typename BufferTraits::allocator_type> buffer_;	///< Buffer of size 2 * P_pad_
	Sample* const x_;		///< excitation vector \f$\mathbf{x}(n)\f$ (P_)
	Sample* const h_;		///< estimated filter response vector \f$\hat{\mathbf{h}}(n)\f$ (P_)
	Sample mu_;				///< LMS algorithm step size \f$\mu\f$
	Sample lambda_;			///< leakage facter (1 - no leakage)

public:

	/// @brief Access to the estimated filter response sequence \f$\hat{h}(n)\f$.
	ioport_rw<const_iterator, iterator> h;
};

/// @brief Implementation of LMS adaptive filter algorithm functor.
/// @see filter_sample_adapt_lms() for explanation of maths under cover.
/// @tparam Sample type of samples of processed signals.
template<class Sample, class BufferTraits = dsp::buffer_traits<Sample> >
class filter_adapt_lms: public lms_filter_base<Sample, BufferTraits> {
	typedef lms_filter_base<Sample, BufferTraits> base;
public:
	/// @brief Initialize LMS algorithm functor.
	/// @param[in] P order of the LMS adaptive filter.
	/// @param[in] mu step size \f$\mu\f$ of the LMS algorithm.
	/// @param[in] lambda leakage factor (1 - no leakage, dafault).
	/// @param[in] initial_h override initial filter response estimate with specified vector of length P.
	filter_adapt_lms(const size_t P, const Sample mu, const Sample lambda = Sample(1), const Sample* initial_h = NULL)
	 :	base(P, mu, lambda, initial_h)
    {}

	/// @brief Perform single step of LMS algorithm, storing estimated system response in internal vector [response_begin(), response_end()).
	/// @see filter_sample_adapt_lms()
	/// @param[in] x excitation signal sample \f$x(n)\f$.
	/// @param[in] d observed signal sample \f$d(n)\f$.
	/// @return estimation error \f$e(n)\f$.
	Sample operator()(const Sample x, const Sample d)
	{
		delay(base::x_, base::P_);
		*base::x_ = x;
		return filter_sample_adapt_lms(d, base::x_, base::h_, base::P_, base::mu_, base::lambda_);
	}
};

/// @brief Implementation of NLMS adaptive filter algorithm functor.
/// @see filter_sample_adapt_nlms() for explanation of maths under cover.
/// @tparam Sample type of samples of processed signals.
template<class Sample, class BufferTraits = dsp::buffer_traits<Sample> >
class filter_adapt_nlms: public lms_filter_base<Sample, BufferTraits> {
	typedef lms_filter_base<Sample, BufferTraits> base;
public:
	/// @brief Initialize NLMS algorithm functor.
	/// @param[in] P order of the NLMS adaptive filter.
	/// @param[in] mu step size \f$\mu\f$ of the NLMS algorithm.
	/// @param[in] gamma denominator offset \f$\gamma\f$ preventing divide by zero.
	/// @param[in] lambda leakage factor (1 - no leakage, dafault).
	/// @param[in] initial_h override initial filter response estimate with specified vector of length P.
	filter_adapt_nlms(const size_t P, const Sample mu, const Sample gamma = Sample(), const Sample lambda = Sample(1), const Sample* initial_h = NULL)
	 :	base(P, mu, lambda, initial_h)
	 ,	gamma_(gamma)
	{}

	/// @brief Perform single step of NLMS algorithm, storing estimated system response in internal vector [response_begin(), response_end()).
	/// @see filter_sample_adapt_nlms()
	/// @param[in] x excitation signal sample \f$x(n)\f$.
	/// @param[in] d observed signal sample \f$d(n)\f$.
	/// @return estimation error \f$e(n)\f$.
	Sample operator()(const Sample x, const Sample d)
	{
		delay(base::x_, base::P_);
		*base::x_ = x;
		return filter_sample_adapt_nlms(d, base::x_, base::h_, base::P_, base::mu_, gamma_, base::lambda_);
	}

	/// @return Correction (offset) coefficient \f$\gamma\f$ safeguarding against division-by-zero on zero input.
	Sample gamma() const {return gamma_;}
	/// @return Correction (offset) coefficient \f$\gamma\f$ safeguarding against division-by-zero on zero input.
	Sample offset() const { return gamma_; }
	/// @brief Modify correction coefficient \f$\gamma\f$ of the NLMS algorithm. @param[in] gamma new gamma used during subsequent iterations.
	void set_gamma(const Sample gamma) {gamma_ = gamma;}
	/// @brief Modify correction coefficient \f$\gamma\f$ of the NLMS algorithm. @param[in] gamma new gamma used during subsequent iterations.
	void set_offset(const Sample gamma) { gamma_ = gamma; }

private:
	Sample gamma_;
};

}

#endif // DSP_ADAPTFILT_H_INCLUDED
