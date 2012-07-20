/*!
 * @file dsp++/filter.h
 * @brief Implementation of "canonical", time-domain filtering based on difference equation.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_FILTER_H_INCLUDED
#define DSP_FILTER_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/utility.h>
#include <dsp++/trivial_array.h>
#include <dsp++/algorithm.h>

#include <algorithm>
#include <functional>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif

namespace dsp {

/*!
 * @brief Filter a single sample x with a Direct-Form II AR/MA digital filter of order P.
 * @param[in] x input signal sample.
 * @param[in,out] w delay line buffer of length P.
 * @param[in] P filter order + 1 (max(N, M)) and the length of delay line used to store intermediate values between subsequent runs.
 * @param[in] a AR filter (IIR) coefficients vector (difference equation denominator), a[0] is assumed to be 1 (coefficients are normalized).
 * @param[in] N number of AR coefficients (length of a vector).
 * @param[in] b MA filter (FIR) coefficients vector (difference equation numerator).
 * @param[in] M number of MA coefficients (length of b vector).
 * @return filtered sample.
 */
template<class Sample> inline
Sample filter_sample_df2(Sample x, Sample* w, const size_t P, const Sample* b, const size_t M, const Sample* a, const size_t N)
{
	delay(w, P);
	*w = x;
	++a;
	for (size_t i = 1; i < N; ++i, ++a)
		*w -= (*a) * w[i];

	x = Sample();
	for (size_t i = 0; i < M; ++i, ++b)
		x += (*b) * w[i];
	return x;
}

const size_t sos_length = 3; //!< Length of coefficient vector of a single second-order-section (SOS) filter.

/*!
 * @brief Filter a single sample x with a bank of N second-order-section IIR filters.
 * @param[in] x input signal sample.
 * @param[in] N number of second-order-sections and the length of w, b, blens, a and alens arrays.
 * @param[in,out] w delay line buffers for each SOS (N).
 * @param[in] b MA (FIR) coefficients of each SOS (N).
 * @param[in] blens number of important samples for each row of b array (N).
 * @param[in] a AR (IIR) coefficients of each SOS (N).
 * @param[in] alens number of important samples for each row of a array (N).
 * @return filtered sample.
 */
template<class Sample> inline
Sample filter_sample_sos_df2(Sample x, size_t N, Sample (*w)[sos_length], const Sample (*b)[sos_length], const size_t* blens, const Sample (*a)[sos_length], const size_t* alens)
{
	for (size_t n = 0; n < N; ++n, ++b, ++blens, ++a, ++alens, ++w)
		x = filter_sample_df2(x, *w, std::max(*alens, *blens), *b, *blens, *a, *alens);
	return x;
}


/*!
 * @brief Implementation of Direct-Form II digital filter.
 */
template<class Sample>
class filter: public sample_based_transform<Sample>
{
public:
	Sample operator()(Sample x);

	template<class BIterator, class AIterator>
	filter(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end);

	template<class BIterator>
	filter(BIterator b_begin, BIterator b_end);

	template<class BSample, class ASample>
	filter(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len);

	/*!
	 * @param b_vec vector of b_len FIR filter coefficients.
	 * @param b_len number of FIR filter coefficients.
	 */
	template<class BSample>
	filter(const BSample* b_vec, size_t b_len);

private:
	const size_t N_; 				//!< Number of AR coefficients.
	const size_t M_;				//!< Number of MA coefficients.
	const size_t P_;				//!< filter order + 1 (max(N_, M_))
	trivial_array<Sample> buffer_;	//!< Buffer of size P_ + N_ + M_
	Sample* const w_;				//!< delay line (P_)
	Sample* const a_;				//!< AR coefficients
	Sample* const b_;				//!< MA coefficients
};

template<class Sample>
Sample filter<Sample>::operator()(Sample x)
{
	return filter_sample_df2(x, w_, P_, b_, M_, a_, N_);
}

template<class Sample>
template<class BIterator, class AIterator>
filter<Sample>::filter(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end)
 :	N_(std::distance(a_begin, a_end))
 ,	M_(std::distance(b_begin, b_end))
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_)
 ,	w_(buffer_.get())
 ,	a_(w_ + P_)
 ,	b_(a_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<AIterator, Sample>));
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<BIterator, Sample>));
#endif

	std::copy(a_begin, a_end, a_);
	std::copy(b_begin, b_end, b_);
	std::transform(a_, a_ + N_, a_, std::bind2nd(std::divides<Sample>(), *a_));
	std::transform(b_, b_ + M_, b_, std::bind2nd(std::divides<Sample>(), *a_));
}

template<class Sample>
template<class BIterator>
filter<Sample>::filter(BIterator b_begin, BIterator b_end)
 :	N_(0)
 ,	M_(std::distance(b_begin, b_end))
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_)
 ,	w_(buffer_.get())
 ,	a_(w_ + P_)
 ,	b_(a_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<BIterator, Sample>));
#endif

	std::copy(b_begin, b_end, b_);
}

template<class Sample>
template<class BSample, class ASample>
filter<Sample>::filter(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len)
 :	N_(a_len)
 ,	M_(b_len)
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_)
 ,	w_(buffer_.get())
 ,	a_(w_ + P_)
 ,	b_(a_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<BSample, Sample>));
	BOOST_CONCEPT_ASSERT((boost::Convertible<ASample, Sample>));
#endif

	std::copy(a_vec, a_vec + a_len, a_);
	std::copy(b_vec, b_vec + b_len, b_);
	Sample a = (0 != a_len ? *a_vec : Sample(1));
	std::transform(a_, a_ + N_, a_, std::bind2nd(std::divides<Sample>(), a));
	std::transform(b_, b_ + M_, b_, std::bind2nd(std::divides<Sample>(), a));
}

template<class Sample>
template<class BSample>
filter<Sample>::filter(const BSample* b_vec, size_t b_len)
 :	N_(0)
 ,	M_(b_len)
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_)
 ,	w_(buffer_.get())
 ,	a_(w_ + P_)
 ,	b_(a_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<BSample, Sample>));
#endif

	std::copy(b_vec, b_vec + b_len, b_);
}

template<class Sample>
class filter_sos: public sample_based_transform<Sample>
{
public:
	static const size_t section_length = sos_length;
	typedef Sample second_order_section[section_length];

	Sample operator()(Sample x);

	template<class CoeffSample, class CoeffSize>
	filter_sos(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl);

private:
	const size_t N_;	//!< number of second-order sections
	trivial_array<Sample> rbuf_;
	trivial_array<size_t> 				lbuf_;
	second_order_section* const num_;
	second_order_section* const den_;
	second_order_section* const w_;
	size_t* const numl_;
	size_t* const denl_;
};

template<class Sample> inline
Sample filter_sos<Sample>::operator ()(Sample x)
{
	return filter_sample_sos_df2(x, N_, w_, num_, numl_, den_, denl_);
}

template<class Sample>
template<class CoeffSample, class CoeffSize> inline
filter_sos<Sample>::filter_sos(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl)
 :	N_(N)
 ,	rbuf_(3 * section_length * N_)
 ,	lbuf_(2 * N_)
 ,	num_(reinterpret_cast<second_order_section*>(rbuf_.get()))
 ,	den_(num_ + N_)
 ,	w_(den_ + N_)
 ,	numl_(lbuf_.get())
 ,	denl_(numl_ + N_)
{
	std::copy(num, num + N, num_);
	std::copy(den, den + N, den_);
	std::copy(numl, numl + N, numl_);
	std::copy(denl, denl + N, denl_);
}

}

#endif /* DSP_FILTER_H_INCLUDED */
