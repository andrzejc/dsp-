/*!
 * @file dsp++/lpc.h
 * @brief Implementation of class lpc (Linear Predictive Coding)
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_LPC_H_INCLUDED
#define DSP_LPC_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/fft.h>
#include <dsp++/complex.h>
#include <dsp++/trivial_array.h>
#include <dsp++/levinson.h>
#include <dsp++/pow2.h>
#include <dsp++/algorithm.h>

#include <stdexcept>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif //DSP_BOOST_CONCEPT_CHECKS_DISABLED

namespace dsp {

/*!
 * @brief Linear Predictive Coding implementation.
 * LPC algorithm determines the coefficients of a forward linear predictor by minimizing the prediction
 * error in least squares sense. LPC uses autocorrelation method of autoregressive modeling to find the
 * filter (predictor) coefficients. For this purpose autocorrelation sequence is calculated (in the
 * frequency domain, with the use of FFT) and Levinson-Durbin recursion is performed.
 * @tparam Sample type of signal samples this algorithm operates on.
 * @tparam DFT type of DFT/IDFT algorithm implementation, defaults to dsp::fft, but may be used with
 * dsp::fftw::dft as well.
 * @see http://en.wikipedia.org/wiki/Linear_predictive_coding
 * @see http://www.mathworks.com/help/toolbox/signal/ref/lpc.html
 */
template<class Sample, template <class, class> class DFT = dsp::fft>
class lpc
{
	typedef typename dsp::remove_complex<Sample>::type real_t;
	typedef typename std::complex<real_t> complex_t;
	typedef DFT<Sample, complex_t> dft_t;
	typedef DFT<complex_t, Sample> idft_t;
	typedef dsp::trivial_array<Sample, typename dft_t::input_allocator> sample_buffer_t;
	typedef dsp::trivial_array<complex_t, typename dft_t::output_allocator> complex_buffer_t;

	static size_t verify_length(const size_t L);
	size_t verify_order(const size_t P);
	size_t verify_dft_length(const size_t dft_size, const size_t idft_size);
public:
	typedef Sample value_type;
	typedef Sample* iterator;
	typedef const Sample* const_iterator;

	size_t input_length() const {return L_;}
	size_t output_length() const {return P_ + 1;}

	/*!
	 * @brief Initialize LPC algorithm for operation on input signal frame of length L and the linear predictor of order P.
	 * @param L length of input signal frame.
	 * @param P order of calculated prediction filter polynomial (length of output sequence is P + 1).
	 */
	explicit lpc(size_t L, size_t P = 0);

	/*!
	 * @brief Initialize LPC algorithm for operation using provided DFT/IDFT objects. The transform size for both must be the same.
	 * @param L length of input signal frame (should be not greater than half of the transform length).
	 * @param dft DFT functor used for autocorrelation calculation.
	 * @param idft IDFT functor used for autocorrelation calculation.
	 * @param P order of calculated prediction filter polynomial (length of output sequence is P + 1).
	 */
	explicit lpc(size_t L, const dft_t& dft, const idft_t& idft, size_t P = 0);

	template<class XIterator, class AIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::OutputIterator<AIterator, Sample>)),
			(Sample))
#else
			Sample
#endif
	operator()(XIterator x, AIterator a)
	{return do_calc(&x, a, static_cast<AIterator*>(NULL));}

	template<class XIterator, class AIterator, class KIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::OutputIterator<AIterator, Sample>))
			((boost::OutputIterator<KIterator, Sample>)),
			(Sample))
#else
			Sample
#endif
	operator()(XIterator x, AIterator a, KIterator k)
	{return do_calc(&x, a, &k);}

	iterator input_begin() {return in_out_.get();}
	iterator input_end() {return in_out_.get() + input_length();}

private:
	template<class XIterator, class AIterator, class KIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::InputIterator<XIterator>))
			((boost::OutputIterator<AIterator, Sample>))
			((boost::OutputIterator<KIterator, Sample>)),
			(Sample))
#else
			Sample
#endif
	do_calc(XIterator* x_begin, AIterator a_begin, KIterator* k_begin)
	{
		const Sample zero = Sample();
		Sample* x = in_out_.get();
		if (NULL != x_begin) dsp::copy_n(*x_begin, L_, x);
		std::fill_n(x + L_, N_ - L_, zero);

		dft_();
		complex_t* c = interm_.get();
		for (size_t i = 0; i < N_; ++i, ++c) *c = std::pow(std::abs(*c), 2);
		idft_();

		std::transform(x, x + N_, x, std::bind2nd(std::divides<Sample>(), static_cast<Sample>(L_ * N_)));
		if (NULL != k_begin)
			return lev_(x, a_begin, *k_begin);
		else
			return lev_(x, a_begin);
	}

	const size_t L_;					//!< input sequence length
	const size_t N_;		 			//!< DFT/IDFT transform length (nextpow2(L_ * 2 - 1))
	const size_t P_;					//!< prediction order
	sample_buffer_t in_out_;
	complex_buffer_t interm_;
	dft_t dft_;
	idft_t idft_;
	dsp::levinson<Sample> lev_;

};

template<class Sample, template <class, class> class DFT> inline
size_t lpc<Sample, DFT>::verify_length(const size_t L)
{
	if (0 == L)
		throw std::domain_error("dsp::lpc input length must be positive");
	return L;
}

template<class Sample, template <class, class> class DFT> inline
size_t lpc<Sample, DFT>::verify_order(const size_t P)
{
	if (0 == P)
		return L_ - 1;
	if (P >= L_)
		throw std::domain_error("dsp::lpc prediction order must not be greater than input length");
	return P;
}

template<class Sample, template <class, class> class DFT> inline
size_t lpc<Sample, DFT>::verify_dft_length(const size_t dft_size, const size_t idft_size)
{
	if (dft_size != idft_size)
		throw std::logic_error("dsp::lpc forward/inverse DFT transform size mismatch");
	if (dft_size < 2 * L_ - 1)
		throw std::domain_error("dsp:lpc DFT transform length too short for given input frame size");
	return dft_size;
}

template<class Sample, template <class, class> class DFT> inline
lpc<Sample, DFT>::lpc(size_t L, size_t P)
 :	L_(verify_length(L))
 ,	N_(dsp::nextpow2(L_ * 2 - 1))
 ,	P_(verify_order(P))
 ,	in_out_(N_)
 ,	interm_(N_)
 ,	dft_(N_, in_out_.get(), interm_.get(), dsp::dft_sign_forward)
 ,	idft_(N_, interm_.get(), in_out_.get(), dsp::dft_sign_backward)
 ,	lev_(N_, P_)
{
}

template<class Sample, template <class, class> class DFT> inline
lpc<Sample, DFT>::lpc(size_t L, const dft_t& dft, const idft_t& idft, size_t P)
 :	L_(verify_length(L))
 ,	N_(verify_dft_length(dft.size(), idft.size()))
 ,	P_(verify_order(P))
 ,	in_out_(N_)
 ,	interm_(N_)
 ,	dft_(dft)
 ,	idft_(idft)
 ,	lev_(N_, P_)
{
}

}

#endif /* DSP_LPC_H_INCLUDED */
