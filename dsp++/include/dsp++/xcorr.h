/*!
 * @file dsp++/xcorr.h
 */
#ifndef DSP_XCORR_H_INCLUDED
#define DSP_XCORR_H_INCLUDED

#include <dsp++/fft.h>
#include <dsp++/complex.h>
#include <dsp++/trivial_array.h>
#include <dsp++/pow2.h>
#include <dsp++/algorithm.h>
#include <dsp++/export.h>

#include <algorithm>

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>


namespace dsp {

class DSPXX_API xcorr_base
{
	static size_t verify_input_length(size_t M);
	size_t verify_transform_length(const size_t dft_size, const size_t idft_size);

public:

	enum mode_type {
		mode_autocorrelation,
		mode_crosscorrelation,
	};

	enum scaling_type {
		scaling_none,
		scaling_biased,
		scaling_unbiased,
		scaling_coeff,
	};

	mode_type mode() const {return (0 == L_ ? mode_autocorrelation : mode_crosscorrelation);}
	size_t x_length() const {return M_;}
	size_t y_length() const {return L_;}
	size_t length() const {return 2 * std::max(M_, L_) - 1;}

protected:
	xcorr_base(size_t M, size_t L)
	 :	M_(verify_input_length(M))
	 ,	L_(L)
	 ,	N_(dsp::nextpow2(std::max(M_, L_) * 2 - 1))
	{
	}

	xcorr_base(size_t M, size_t L, size_t dft_size, size_t idft_size)
	 :	M_(verify_input_length(M))
	 ,	L_(L)
	 ,	N_(verify_transform_length(dft_size, idft_size))
	{
	}

	size_t M_; 			//!< length of input sequence x, must be > 0
	size_t L_;			//!< length of input sequence y (if 0 this is autocorrelation).
	size_t N_;			//!< transform length (nextpow2(max(M_, L_) * 2 - 1));
};

template<class Sample, template <class, class> class DFT = dsp::fft>
class xcorr: public xcorr_base
{
	typedef typename dsp::remove_complex<Sample>::type real_t;
	typedef typename std::complex<real_t> complex_t;
	typedef DFT<Sample, complex_t> dft_t;
	typedef DFT<complex_t, Sample> idft_t;
	typedef dsp::trivial_array<Sample, typename dft_t::input_allocator> sample_buffer_t;
	typedef dsp::trivial_array<complex_t, typename dft_t::output_allocator> complex_buffer_t;
public:
	typedef Sample value_type;
	typedef Sample* iterator;
	typedef const Sample* const_iterator;


	iterator x_begin() {return x_.get();}
	const_iterator x_begin() const {return x_.get();}
	iterator x_end() {return x_.get() + M_;}
	const_iterator x_end() const {return x_.get() + M_;}

	iterator y_begin() {return x_.get() + N_;}
	const_iterator y_begin() const {return x_.get() + N_;}
	iterator y_end() {return x_.get() + (N_ + L_);}
	const_iterator y_end() const {return x_.get() + (N_ + L_);}

	const_iterator begin() const {return x_.get();}
	const_iterator end() const {return x_.get() + length();}


	template<class XIterator, class OutputIterator>
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::OutputIterator<OutputIterator, Sample>)),
			(void))
	operator()(XIterator x, OutputIterator out, scaling_type scaling)
	{do_calc(&x, static_cast<XIterator*>(NULL), &out, scaling);}

	template<class XIterator, class YIterator, class OutputIterator>
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::InputIterator<YIterator>))
			((boost::OutputIterator<OutputIterator, Sample>)),
			(void))
	operator()(XIterator x, YIterator y, OutputIterator out, scaling_type scaling)
	{do_calc(&x, &y, &out, scaling);}

	void operator()(scaling_type scaling)
	{do_calc(static_cast<const_iterator*>(NULL), static_cast<const_iterator*>(NULL), static_cast<iterator*>(NULL), scaling);}

	explicit xcorr(size_t M, size_t L = 0);

	xcorr(size_t M, size_t L, const dft_t& dft, const idft_t& idft);

private:
	sample_buffer_t x_;	//!< input/output buffer (2 * N_ if crosscorrelation, N_ + M - 1 if autocorrelation)
	complex_buffer_t X_;	//!< intermediate DFT buffer (2 * N_ if crosscorrelation, N_ if autocorrelation)
	dft_t dft_;
	idft_t idft_;

	template<class XIterator, class YIterator, class OutputIterator>
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::InputIterator<YIterator>))
			((boost::OutputIterator<OutputIterator, Sample>)),
			(void))
	do_calc(XIterator* x, YIterator* y, OutputIterator* out, scaling_type scaling)
	{
		const Sample zero = Sample();
		Sample* xx = x_begin();
		Sample* yy = y_begin();
		if (NULL != x && static_cast<const void*>(&(**x)) != xx) dsp::copy_n(*x, M_, xx);
		std::fill_n(xx + M_, N_ - M_, zero);
		if (NULL != y && static_cast<const void*>(&(**y)) != yy) dsp::copy_n(*y, L_, yy);

		real_t sumx = real_t(), sumy = real_t();
		dft_(xx, X_.get());

		if (0 != L_)
		{
			// this is the cross-correlation case, calculate DFT of both vectors and conjugate-multiply the transforms
			if (scaling_coeff == scaling)
			{
				// the input vectors will be overwritten by the output, so calculate autocorrelation coefficients now
				sumx = acorr_coeff(xx, M_);
				sumy = acorr_coeff(yy, L_);
			}

			std::fill_n(yy + L_, N_ - L_, zero);
			dft_(yy, X_.get() + N_);
			complex_t* X = X_.get();
			complex_t* Y = X + N_;
			for (size_t i = 0; i < N_; ++i, ++X, ++Y) *X *= dsp::conj(*Y);
		}
		else
		{
			// this is autocorrelation case, DFT of single vector is enough
			complex_t* X = X_.get();
			for (size_t i = 0; i < N_; ++i, ++X) *X = std::pow(std::abs(*X), 2);
		}

		idft_();

		const size_t M = std::max(M_, L_);
		const size_t len = length();

		std::copy(xx + (N_ - M + 1), xx + N_, xx + N_);
		std::copy_backward(xx, xx + M, xx + len);
		std::copy(xx + N_, xx + (N_ + M - 1), xx);

		switch (scaling) {
		case scaling_none:
			for (size_t i = 0; i < len; ++i, ++xx) *xx /= N_; // just normalize IDFT output
			break;
		case scaling_biased: {
			const size_t num = N_ * M;
			for (size_t i = 0; i < len; ++i, ++xx) *xx /= num;
			break; }
		case scaling_coeff:
			if (0 == L_) {
				const Sample scale = Sample(1) / *(xx + (M_ - 1));
				for (size_t i = 0; i < len; ++i, ++xx) *xx *= scale;
			}
			else {
				const real_t scale = 1 / (std::sqrt(sumx * sumy) * N_);
				for (size_t i = 0; i < len; ++i, ++xx) *xx *= scale;
			}
			break;
		case scaling_unbiased:
			for (int i = 0; i < static_cast<int>(len); ++i, ++xx)
				*xx /= (N_ * (M_ - std::abs(static_cast<int>(M) - i - 1)));
			break;
		}
		if (NULL != out) dsp::copy_n(begin(), len, *out);
	}


	static real_t acorr_coeff(const Sample* vec, size_t N)
	{
		real_t sum = real_t();
		for (size_t i = 0; i < N; ++i, ++vec)
			sum += std::pow(std::abs(*vec), 2);
		return sum;
	}

};

template<class Sample, template <class, class> class DFT>
xcorr<Sample, DFT>::xcorr(size_t M, size_t L)
 :	xcorr_base(M, L)
 ,	x_(L_ == 0 ? N_ + M_ - 1 : 2 * N_)
 ,	X_(L_ == 0 ? N_ : 2 * N_)
 ,	dft_(N_, x_.get(), X_.get(), dsp::dft_sign_forward)
 ,	idft_(N_, X_.get(), x_.get(), dsp::dft_sign_backward)
{
}

template<class Sample, template <class, class> class DFT>
xcorr<Sample, DFT>::xcorr(size_t M, size_t L, const dft_t& dft, const idft_t& idft)
 :	xcorr_base(M, L, dft.size(), idft.size())
 ,	x_(L_ == 0 ? N_ + M_ - 1 : 2 * N_)
 ,	X_(L_ == 0 ? N_ : 2 * N_)
 ,	dft_(dft)
 ,	idft_(idft)
{
}

}

#endif /* DSP_XCORR_H_INCLUDED */
