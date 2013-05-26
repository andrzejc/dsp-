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
#include <stdexcept>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif

namespace dsp {

/*!
 * @brief Filter a single sample x with a Direct-Form II AR/MA digital filter of order P.
 * @param[in] x input signal sample.
 * @param[in,out] w delay line buffer of length P.
 * @param[in] b MA filter (FIR) coefficients vector (difference equation numerator).
 * @param[in] M number of MA coefficients (length of b vector).
 * @param[in] a AR filter (IIR) coefficients vector (difference equation denominator), a[0] is assumed to be 1 (coefficients are normalized).
 * @param[in] N number of AR coefficients (length of a vector).
 * @return filtered sample.
 */
template<class Sample> inline
Sample filter_sample_df2(Sample x, Sample* w, const Sample* b, const size_t M, const Sample* a, const size_t N)
{
	*w = x;
	++a;
	for (size_t i = 1; i < N; ++i, ++a)
		*w -= (*a) * w[i];

	x = Sample();
	for (size_t i = 0; i < M; ++i, ++b)
		x += (*b) * w[i];
	return x;
}

const size_t sos_length = 3; //!< Length of coefficient vector of a single second-order-section (SOS) filter (3).

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
		x = filter_sample_df2(x, *w, *b, *blens, *a, *alens);
	return x;
}

template<class Sample>
class df2_filter_base
{
public:

	//! @return order of the implemented filter.
	size_t order() const {return P_ - 1;}

	template<class BIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::OutputIterator<BIterator, Sample>)),(void))
#else
	void
#endif
	set(BIterator b_begin, BIterator b_end)
	{
		size_t m = std::distance(b_begin, b_end);
		if (m > M_)
			throw std::length_error("filter length exceeds previous one");
		std::copy(b_begin, b_end, b_);
		M_ = m;
	}

	template<class BIterator, class AIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::OutputIterator<BIterator, Sample>))((boost::OutputIterator<AIterator, Sample>)),(void))
#else
	void
#endif
	set(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end)
	{
		size_t m = std::distance(b_begin, b_end), n = std::distance(a_begin, a_end);
		if (m > M_ || n > N_)
			throw std::length_error("filter length exceeds previous one");
		std::copy(b_begin, b_end, b_);
		std::copy(a_begin, a_end, a_);
		M_ = m;
		N_ = n;
	}

	template<class BSample, class ASample>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::Convertible<BSample, Sample>))((boost::Convertible<ASample, Sample>)),(void))
#else
	void
#endif
	set(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len)
	{
		if (b_len > M_ || a_len > N_)
			throw std::length_error("filter length exceeds previous one");

		std::copy(a_vec, a_vec + a_len, a_);
		std::copy(b_vec, b_vec + b_len, b_);
		Sample a = (0 != a_len ? *a_vec : Sample(1));
		std::transform(a_, a_ + N_, a_, std::bind2nd(std::divides<Sample>(), a));
		std::transform(b_, b_ + M_, b_, std::bind2nd(std::divides<Sample>(), a));
	}

	template<class BSample>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::Convertible<BSample, Sample>)),(void))
#else
	void
#endif
	set(const BSample* b_vec, size_t b_len)
	{
		if (b_len > M_)
			throw std::length_error("filter length exceeds previous one");
		std::copy(b_vec, b_vec + b_len, b_);
	}

protected:

	template<class BIterator, class AIterator>
	df2_filter_base(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end, size_t L);

	template<class BIterator>
	df2_filter_base(BIterator b_begin, BIterator b_end, size_t L);

	template<class BSample, class ASample>
	df2_filter_base(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len, size_t L);

	template<class BSample>
	df2_filter_base(const BSample* b_vec, size_t b_len, size_t L);

	df2_filter_base(size_t N, size_t M, size_t P, size_t L)
	 :	N_(N)
	 ,	M_(M)
	 ,	P_(P)
	 ,	buffer_(P_ + N_ + M_ + L - 1)
	 ,	a_(buffer_.get())
	 ,	b_(a_ + N_)
	 ,	w_(b_ + M_)
	{
	}

	const size_t N_; 				//!< Number of AR coefficients.
	const size_t M_;				//!< Number of MA coefficients.
	const size_t P_;				//!< filter order + 1 (max(N_, M_))
	trivial_array<Sample> buffer_;	//!< Buffer of size P_ + N_ + M_ (+ L_ - 1 in case of block filter)
	Sample* const a_;				//!< AR coefficients
	Sample* const b_;				//!< MA coefficients
	Sample* const w_;				//!< delay line (P_ (+ L_ - 1 in case of block filter))

};

template<class Sample>
template<class BIterator, class AIterator>
df2_filter_base<Sample>::df2_filter_base(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end, size_t L)
 :	N_(std::distance(a_begin, a_end))
 ,	M_(std::distance(b_begin, b_end))
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_ + L - 1)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_)
 ,	w_(b_ + M_)
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
df2_filter_base<Sample>::df2_filter_base(BIterator b_begin, BIterator b_end, size_t L)
 :	N_(0)
 ,	M_(std::distance(b_begin, b_end))
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_ + L - 1)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_)
 ,	w_(b_ + M_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<BIterator, Sample>));
#endif

	std::copy(b_begin, b_end, b_);
}

template<class Sample>
template<class BSample, class ASample>
df2_filter_base<Sample>::df2_filter_base(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len, size_t L)
 :	N_(a_len)
 ,	M_(b_len)
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_ + L - 1)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_)
 ,	w_(b_ + M_)
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
df2_filter_base<Sample>::df2_filter_base(const BSample* b_vec, size_t b_len, size_t L)
 :	N_(0)
 ,	M_(b_len)
 ,	P_(std::max(N_, M_))
 ,	buffer_(P_ + N_ + M_ + L - 1)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_)
 ,	w_(b_ + M_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<BSample, Sample>));
#endif

	std::copy(b_vec, b_vec + b_len, b_);
}

/*!
 * @brief Implementation of Direct-Form II digital filter.
 */
template<class Sample>
class filter: public df2_filter_base<Sample>, public sample_based_transform<Sample>
{
	typedef df2_filter_base<Sample> base;
public:
	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	inline Sample operator()(Sample x);

	/*!
	 * @brief Construct filter given coefficients vectors as iterator ranges [b_begin, b_end) and [a_begin, a_end).
	 * @param b_begin start of numerator coefficients sequence.
	 * @param b_end end of numerator coefficients sequence.
	 * @param a_begin start of denominator coefficients sequence.
	 * @param a_end end of denominator coefficients sequence.
	 * @tparam BIterator type of iterator used to denote numerator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 * @tparam AIterator type of iterator used to denote denominator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 */
	template<class BIterator, class AIterator>
	filter(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end)
	 :	base(b_begin, b_end, a_begin, a_end, 1) {}

	/*!
	 * @brief Construct all-zero filter given coefficients vector as iterator range [b_begin, b_end).
	 * @param b_begin start of numerator coefficients sequence.
	 * @param b_end end of numerator coefficients sequence.
	 * @tparam BIterator type of iterator used to denote numerator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 */
	template<class BIterator>
	filter(BIterator b_begin, BIterator b_end)
	 :	base(b_begin, b_end, 1) {}

	/*!
	 * @brief Construct filter given coefficients vectors as C arrays.
	 * @param b_vec start of numerator coefficients sequence.
	 * @param b_len length numerator coefficients sequence.
	 * @param a_vec start of denominator coefficients sequence.
	 * @param a_len length of denominator coefficients sequence.
	 */
	template<class BSample, class ASample>
	filter(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len)
	 :	base(b_vec, b_len, a_vec, a_len, 1) {}

	/*!
	 * @brief Construct all-zero filter given coefficients vector as C array.
	 * @param b_vec vector of b_len FIR filter coefficients.
	 * @param b_len number of FIR filter coefficients.
	 */
	template<class BSample>
	filter(const BSample* b_vec, size_t b_len)
	 :	base(b_vec, b_len, 1) {}

};

template<class Sample> inline 
Sample filter<Sample>::operator()(Sample x)
{
	delay(base::w_, base::P_);
	return filter_sample_df2(x, base::w_, base::b_, base::M_, base::a_, base::N_);
}

/*!
 * @brief Implementation of Direct-Form II digital filter realized as a bank of second-order-sections (SOS).
 * @tparam Sample type of samples this filter operates on.
 */
template<class Sample>
class filter_sos: public sample_based_transform<Sample>
{
public:
	static const size_t section_length = sos_length; 		//!< Length of coefficient vector of a single second-order-section (SOS) filter (3).
	typedef Sample second_order_section[section_length];	//!< Array used to store SOS samples.

	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	inline Sample operator()(Sample x);

	/*!
	 * @brief Construct SOS-bank filter given coefficients provided as a matrix in a form compatible with
	 * MATLAB fdatool output.
	 * @param N number of second-order-sections (rows in num, numl, den and denl arrays).
	 * @param num @f$N{\times}section\_length@f$ matrix with N rows of numerator coefficients.
	 * @param numl N-row array with lengths of each coefficient vector in matrix num.
	 * @param den @f$N{\times}section\_length@f$ matrix with N rows of denominator coefficients.
	 * @param denl N-row array with lengths of each coefficient vector in matrix den.
	 */
	template<class CoeffSample, class CoeffSize>
	filter_sos(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl);

private:
	const size_t N_;				//!< number of second-order sections
	trivial_array<Sample> rbuf_;
	trivial_array<size_t> lbuf_;
	second_order_section* const num_;
	second_order_section* const den_;
	second_order_section* const w_;
	size_t* const numl_;
	size_t* const denl_;
};

template<class Sample> inline
Sample filter_sos<Sample>::operator ()(Sample x)
{
	for (size_t n = 0; n < N_; ++n)
		delay(w_[n], std::max(numl_[n], denl_[n]));
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
#ifdef _MSC_VER
	for (size_t i = 0; i < N; ++i) {
		Sample (&nto)[section_length] = num_[i];
		const CoeffSample (&nfrom)[section_length] = num[i];
		std::copy(nfrom, nfrom + section_length, nto);

		Sample (&dto)[section_length] = den_[i];
		const CoeffSample (&dfrom)[section_length] = den[i];
		std::copy(dfrom, dfrom + section_length, dto);

		numl_[i] = numl[i];
		denl_[i] = denl[i];
	}
#else
	std::copy(num, num + N, num_);
	std::copy(den, den + N, den_);
	std::copy(numl, numl + N, numl_);
	std::copy(denl, denl + N, denl_);
#endif
}

/*!
 * @brief Implementation of Direct-Form II digital filter.
 */
template<class Sample>
class block_filter: public df2_filter_base<Sample>
{
	typedef df2_filter_base<Sample> base;

public:
	typedef Sample* iterator;
	typedef const Sample* const_iterator;

	/*!
	 * @brief Construct filter given coefficients vectors as iterator ranges [b_begin, b_end) and [a_begin, a_end).
	 * @param L processing block length.
	 * @param b_begin start of numerator coefficients sequence.
	 * @param b_end end of numerator coefficients sequence.
	 * @param a_begin start of denominator coefficients sequence.
	 * @param a_end end of denominator coefficients sequence.
	 * @tparam BIterator type of iterator used to denote numerator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 * @tparam AIterator type of iterator used to denote denominator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 */
	template<class BIterator, class AIterator>
	block_filter(size_t L, BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end)
	 :	base(b_begin, b_end, a_begin, a_end, L * 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	/*!
	 * @brief Construct all-zero filter given coefficients vector as iterator range [b_begin, b_end).
	 * @param L processing block length.
	 * @param b_begin start of numerator coefficients sequence.
	 * @param b_end end of numerator coefficients sequence.
	 * @tparam BIterator type of iterator used to denote numerator sequence, must adhere to OutputIterator concept
	 * with value type convertible to Sample.
	 */
	template<class BIterator>
	block_filter(size_t L, BIterator b_begin, BIterator b_end)
	 :	base(b_begin, b_end, L * 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	/*!
	 * @brief Construct filter given coefficients vectors as C arrays.
	 * @param L processing block length.
	 * @param b_vec start of numerator coefficients sequence.
	 * @param b_len length numerator coefficients sequence.
	 * @param a_vec start of denominator coefficients sequence.
	 * @param a_len length of denominator coefficients sequence.
	 */
	template<class BSample, class ASample>
	block_filter(size_t L, const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len)
	 :	base(b_vec, b_len, a_vec, a_len, L * 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	/*!
	 * @brief Construct all-zero filter given coefficients vector as C array.
	 * @param L processing block length.
	 * @param b_vec vector of b_len FIR filter coefficients.
	 * @param b_len number of FIR filter coefficients.
	 */
	template<class BSample>
	block_filter(size_t L, const BSample* b_vec, size_t b_len)
	 :	base(b_vec, b_len, L + 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	iterator begin() {return x_;}
	iterator end() {return x_ + L_;}
	const_iterator begin() const {return x_ ;}
	const_iterator end() const {return x_ + L_;}

	//! @brief Apply the filter to the sample sequence specified by [begin(), end()) range.
	inline void operator()()
	{
		std::copy(base::w_, base::w_ + base::P_ - 1, base::w_ + L_);
		Sample* w = base::w_ + L_ - 1;
		Sample* x = x_;
		for (size_t n = 0; n != L_; ++n, --w, ++x) 
			*x = filter_sample_df2(*x, w, base::b_, base::M_, base::a_, base::N_);
	}

private:
	const size_t L_;
	Sample* const x_;
};

}

#endif /* DSP_FILTER_H_INCLUDED */
