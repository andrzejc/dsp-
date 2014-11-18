/*!
 * @file dsp++/filter.h
 * @brief Implementation of "canonical", time-domain filtering based on difference equation.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_FILTER_H_INCLUDED
#define DSP_FILTER_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/utility.h>
#include <dsp++/buffer_traits.h>
#include <dsp++/trivial_array.h>
#include <dsp++/algorithm.h>
#include <dsp++/simd.h>

#include <algorithm>
#include <functional>
#include <stdexcept>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif

namespace dsp {

/*!
 * @brief Filter a single input sample with a Direct-Form II AR/MA digital filter of order P.
 * @param[in,out] w delay line buffer of length max(N, M) (== P + 1), on input w[0] should contain sample to be filtered x[0], upon output it will be output of IIR section.
 * @param[in] b MA filter (FIR) coefficients vector (difference equation numerator).
 * @param[in] M number of MA coefficients (length of b vector).
 * @param[in] a AR filter (IIR) coefficients vector (difference equation denominator), a[0] is assumed to be 1 (coefficients are normalized).
 * @param[in] N number of AR coefficients (length of a vector).
 * @return filtered sample.
 */
template<class Sample> inline
Sample filter_sample_df2(Sample* w, const Sample* b, const size_t M, const Sample* a, const size_t N)
{
	++a; 	// we assume a[0] is 1 and skip the multiplication
	const Sample* y = w + 1;
	for (size_t i = 1; i < N; ++i, ++a, ++y)
		*w -= (*a) * (*y);

	Sample r = Sample();
	y = w;
	for (size_t i = 0; i < M; ++i, ++b, ++y)
		r += (*b) * (*y);
	return r;
}

const size_t sos_length = 3; //!< Length of coefficient vector of a single second-order-section (SOS) filter (3).

/*!
 * @brief Filter a single input sample through a cascade of Second-order Sections, optimized with SIMD instructions.
 * @param[in] x input signal sample.
 * @param[in] N number of sections in a cascade.
 * @param[in] scale_only array of bools which indicate (if true), that this section contains only b0 coefficient (is 0-th order scaler) (N).
 * @param[in,out] w vector used for storing intermediate calculation results - N step-length delay line vectors (step * N).
 * @param[in] b vector with MA coefficients for each section (step * N)
 * @param[in] a vector with AR coefficients for each section (step * N)
 * @param[in] step length of each section coefficient and delay line subvectors.
 */
template<class Sample> inline
Sample filter_sample_sos_df2(Sample x, size_t N, const bool* scale_only, Sample* w, const Sample* b, const Sample* a, size_t step)
{
	for (size_t j = 0; j < N; ++j, ++scale_only, w += step, b += step, a += step) {
		if (*scale_only)
			x *= *b;
		else {
			Sample sum = Sample();
			for (size_t i = 1; i < sos_length; ++i)
				sum += a[i] * w[i];
			*w = x - sum;
			sum = Sample();
			for (size_t i = 0; i < sos_length; ++i)
				sum += b[i] * w[i];
			x = sum;
		}
	}
	return x;
}


namespace simd {

/*!
 * @brief Filter a single input sample with a Direct-Form II AR/MA digital filter of order P using SIMD instructions.
 * @param[in,out] w Delay line buffer of length max(M, N), needs not to be aligned (we use unaligned reads here as it should
 * be generally faster to read/write unaligned sliding window than move the memory block in each iteration).
 * @param[in] b FIR filter coefficients vector, must be aligned and padded (M)
 * @param[in] M Padded length of b vector.
 * @param[in] a IIR filter coefficients vector, must be aligned and padded, a[0] must be set to 0 for efficiency reasons (N)
 * @param[in] N Padded length of a vector.
 * @note M and N must include padding.
 * @note a[0] must be set on input to 0, although it is assumed to be 1.
 * @return filtered sample.
 */
DSPXX_API float filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N);
/*!
 * @param[in] feat_flags override runtime CPU feature flags detection and run as if the specified features were present.
 * @copydoc filter_sample_df2(float*, const float*, const size_t, const float*, const size_t)
 */
DSPXX_API float filter_sample_df2(float* w, const float* b, const size_t M, const float* a, const size_t N, int feat_flags);

/*!
 * @brief Filter a single input sample through a cascade of Second-order Sections, optimized with SIMD instructions.
 * @param[in] x input sample.
 * @param[in] N number of sections in a cascade.
 * @param[in] scale_only array of bools which indicate (if true), that this section contains only b0 coefficient (is 0-th order scaler) (N).
 * @param[in,out] w vector used for storing intermediate calculation results - delay line of (padded) length step for each section, must be
 * properly padded and aligned according to dsp::simd::aligned_count<float>(sos_length) (step * N).
 * @param[in] b vector with MA coefficients for each section (step * N)
 * @param[in] a vector with AR coefficients for each section (step * N)
 * @param[in] step length of each section coefficient and delay line subvectors (basically result of dsp::simd::aligned_count<float>(sos_length) call).
 * @note a[step * {0, 1, ... N-1}] must be set on input to 0, although it is assumed to be 1.
 */
DSPXX_API float filter_sample_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step);
/*!
 * @param[in] feat_flags override runtime CPU feature flags detection and run as if the specified features were present.
 * @copydoc filter_sample_sos_df2(float, size_t, const bool*, float*, const float*, const float*, size_t)
 */
DSPXX_API float filter_sample_sos_df2(float x, size_t N, const bool* scale_only, float* w, const float* b, const float* a, size_t step, int feat_flags);

}

template<class Sample, class BufferTraits = dsp::buffer_traits<Sample> >
class df2_filter_base
{
	/*!
	 * @brief Normalize numerator and denominator by a[0] and set it to 0 as required by SIMD functions.
	 */
	void normalize() {
		if (0 == N_)
			return;
		if (Sample(1) != *a_) {
			std::transform(b_, b_ + M_, b_, std::bind2nd(std::divides<Sample>(), *a_));
			std::transform(a_, a_ + N_, a_, std::bind2nd(std::divides<Sample>(), *a_));
		}
		*a_ = Sample();
	}

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
		const size_t m = std::distance(b_begin, b_end);
		if (m > M_)
			throw std::length_error("filter length exceeds declared one");
		std::fill_n(b_ + m, b_ + M_, Sample());
		std::fill_n(a_, a_ + N_, Sample());
		std::copy(b_begin, b_end, b_);
		M_ = m; M_pad_ = BufferTraits::aligned_count(M_);
		N_ = 0; N_pad_ = 0;
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
			throw std::length_error("filter length exceeds declared one");

		std::fill_n(b_ + m, b_ + M_, Sample());
		std::copy(b_begin, b_end, b_);
		std::fill_n(a_ + n, a_ + N_, Sample());
		std::copy(a_begin, a_end, a_);
		M_ = m; M_pad_ = BufferTraits::aligned_count(M_);
		N_ = n; N_pad_ = BufferTraits::aligned_count(N_);
		normalize();
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
			throw std::length_error("filter length exceeds declared one");

		std::fill_n(b_ + b_len, b_ + M_, Sample());
		std::copy(b_vec, b_vec + b_len, b_);
		std::fill_n(a_ + a_len, a_ + N_, Sample());
		std::copy(a_vec, a_vec + a_len, a_);
		M_ = b_len; M_pad_ = BufferTraits::aligned_count(M_);
		N_ = a_len; N_pad_ = BufferTraits::aligned_count(N_);
		normalize();
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
			throw std::length_error("filter length exceeds declared one");
		std::fill_n(b_ + b_len, b_ + M_, Sample());
		std::fill_n(a_, a_ + N_, Sample());
		std::copy(b_vec, b_vec + b_len, b_);
		M_ = b_len; M_pad_ = BufferTraits::aligned_count(M_);
		N_ = 0; N_pad_ = 0;
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
	 :	N_(N), N_pad_(BufferTraits::aligned_count(N_))
	 ,	M_(M), M_pad_(BufferTraits::aligned_count(M_))
	 ,	P_(P), W_pad_(BufferTraits::aligned_count(P_ + L -1))
	 ,	buffer_(N_pad_ + M_pad_ + W_pad_)
	 ,	a_(buffer_.get())
	 ,	b_(a_ + N_pad_)
	 ,	w_(b_ + M_pad_)
	{
	}

	const size_t N_; 				//!< Number of AR coefficients.
	const size_t N_pad_;			//!< Length of a_ vector with padding included.
	const size_t M_;				//!< Number of MA coefficients.
	const size_t M_pad_;			//!< Length of b_ vector with padding included.
	const size_t P_;				//!< filter order + 1 (max(N_, M_))
	const size_t W_pad_;			//!< Length of w_ vector with padding included.
	trivial_array<Sample, typename BufferTraits::allocator_type> buffer_;	//!< Buffer of size P_ + N_ + M_ (+ L_ - 1 in case of block filter)
	Sample* const a_;				//!< AR coefficients
	Sample* const b_;				//!< MA coefficients
	Sample* const w_;				//!< delay line (P_ (+ L_ - 1 in case of block filter))

};

template<class Sample, class BufferTraits>
template<class BIterator, class AIterator>
df2_filter_base<Sample, BufferTraits>::df2_filter_base(BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end, size_t L)
 :	N_(std::distance(a_begin, a_end)), N_pad_(BufferTraits::aligned_count(N_))
 ,	M_(std::distance(b_begin, b_end)), M_pad_(BufferTraits::aligned_count(M_))
 ,	P_(std::max(N_, M_)), W_pad_(BufferTraits::aligned_count(P_ + L - 1))
 ,	buffer_(N_pad_ + M_pad_ + W_pad_)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_pad_)
 ,	w_(b_ + M_pad_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::InputIterator<AIterator>));
	BOOST_CONCEPT_ASSERT((boost::InputIterator<BIterator>));
#endif

	std::copy(a_begin, a_end, a_);
	std::copy(b_begin, b_end, b_);
	normalize();
}

template<class Sample, class BufferTraits>
template<class BIterator>
df2_filter_base<Sample, BufferTraits>::df2_filter_base(BIterator b_begin, BIterator b_end, size_t L)
 :	N_(0), N_pad_(0)
 ,	M_(std::distance(b_begin, b_end)), M_pad_(BufferTraits::aligned_count(M_))
 ,	P_(M_), W_pad_(BufferTraits::aligned_count(P_ + L - 1))
 ,	buffer_(N_pad_ + M_pad_ + W_pad_)
 ,	a_(buffer_.get())
 ,	b_(a_)
 ,	w_(b_ + M_pad_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::InputIterator<BIterator>));
#endif

	std::copy(b_begin, b_end, b_);
}

template<class Sample, class BufferTraits>
template<class BSample, class ASample>
df2_filter_base<Sample, BufferTraits>::df2_filter_base(const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len, size_t L)
 :	N_(a_len), N_pad_(BufferTraits::aligned_count(N_))
 ,	M_(b_len), M_pad_(BufferTraits::aligned_count(M_))
 ,	P_(std::max(N_, M_)), W_pad_(BufferTraits::aligned_count(P_ + L - 1))
 ,	buffer_(N_pad_ + M_pad_ + W_pad_)
 ,	a_(buffer_.get())
 ,	b_(a_ + N_pad_)
 ,	w_(b_ + M_pad_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<BSample, Sample>));
	BOOST_CONCEPT_ASSERT((boost::Convertible<ASample, Sample>));
#endif

	std::copy(a_vec, a_vec + a_len, a_);
	std::copy(b_vec, b_vec + b_len, b_);
	normalize();
}

template<class Sample, class BufferTraits>
template<class BSample>
df2_filter_base<Sample, BufferTraits>::df2_filter_base(const BSample* b_vec, size_t b_len, size_t L)
 :	N_(0), N_pad_(0)
 ,	M_(b_len), M_pad_(BufferTraits::aligned_count(M_))
 ,	P_(M_), W_pad_(BufferTraits::aligned_count(P_ + L - 1))
 ,	buffer_(N_pad_ + M_pad_ + W_pad_)
 ,	a_(buffer_.get())
 ,	b_(a_)
 ,	w_(b_ + M_pad_)
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

	/*!
	 * @brief Prepare filter for operation with given number of coefficients without actually initializing them.
	 * @param[in] b_len number of numerator (MA) coefficients
	 * @param[in] a_len number of denominator (AR) coefficients
	 * @see use set() to actually initialize to coefficient values
	 */
	explicit filter(size_t b_len, size_t a_len)
		:	base(a_len, b_len, std::max(a_len, b_len), 1) {}

	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	Sample operator()(Sample x)
	{
		delay(base::w_, base::P_);
		*base::w_ = x;
		return filter_sample_df2(base::w_, base::b_, base::M_, base::a_, base::N_);
	}
};

template<>
class DSPXX_API filter<float>: public df2_filter_base<float, dsp::simd::buffer_traits<float> >, public sample_based_transform<float>
{
	typedef df2_filter_base<float, dsp::simd::buffer_traits<float> > base;
public:
	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	inline float operator()(float x) {
		delay(w_, P_);
		*w_ = x;
		return dsp::simd::filter_sample_df2(w_, b_, M_pad_, a_, N_pad_);
	}

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

	/*!
	 * @brief Prepare filter for operation with given number of coefficients without actually initializing them.
	 * @param[in] b_len number of numerator (MA) coefficients
	 * @param[in] a_len number of denominator (AR) coefficients
	 * @see use set() to actually initialize to coefficient values
	 */
	explicit filter(size_t b_len, size_t a_len)
 	 :	base(a_len, b_len, std::max(a_len, b_len), 1) {}
};


template<class Sample, class BufferTraits = dsp::buffer_traits<Sample> >
class sos_filter_base
{
protected:
	static const size_t step_min = sos_length + 1;
	static size_t step() {return BufferTraits::aligned_count(step_min);}
public:
	static const size_t section_length = sos_length; 		//!< Length of coefficient vector of a single second-order-section (SOS) filter (3).

	size_t section_count() const {return N_;}

	template<class CoeffSample, class CoeffSize>
	void set(const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl);

	template<class CoeffSample>
	void set(const CoeffSample* num, const CoeffSample* den);

protected:
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
	sos_filter_base(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl);

	template<class CoeffSample>
	sos_filter_base(size_t N, const CoeffSample* num, const CoeffSample* den);

	/*!
	 * @brief Prepare filter for operation with given number of sections without actually initializing them.
	 * @param[in] N number of second order sections
	 * @see use set() to actually initialize to coefficient values
	 */
	explicit sos_filter_base(size_t N);

	const size_t step_;
	const size_t N_;				//!< number of second-order sections
	trivial_array<Sample, typename BufferTraits::allocator_type> rbuf_;
	trivial_array<bool>   scale_only_;
	Sample* const b_;
	Sample* const a_;
	Sample* const w_;
};

template<class Sample, class BufferTraits>
template<class CoeffSample, class CoeffSize> inline
void sos_filter_base<Sample, BufferTraits>::set(const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<CoeffSample, Sample>));
	BOOST_CONCEPT_ASSERT((boost::Convertible<CoeffSize, size_t>));
#endif
	Sample* b = b_;
	Sample* a = a_;
	for (size_t i = 0; i < N_; ++i, b += step_, a += step_, ++num, ++numl, ++den, ++denl) {
		std::copy(*num, *num + *numl, b);
		std::copy(*den, *den + *denl, a);
		if (*denl > 0 && Sample(1) != *a) {
			std::transform(b, b + *numl, b, std::bind2nd(std::divides<Sample>(), *a));
			std::transform(a, a + *denl, a, std::bind2nd(std::divides<Sample>(), *a));
		}
		*a = Sample(); // set a[0] to 0 as an optimization for dot product calculation
		scale_only_[i] = (*numl == 1) && (*denl <= 1);
	}
}

template<class Sample, class BufferTraits>
template<class CoeffSample, class CoeffSize> inline
sos_filter_base<Sample, BufferTraits>::sos_filter_base(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl)
 :	step_(step())
 , 	N_(N)
 ,	rbuf_(3 * step_ * N_)
 ,	scale_only_(N_)
 ,	b_(rbuf_.get())
 ,	a_(b_ + N_ * step_)
 ,	w_(a_ + N_ * step_)
{
	set(num, numl, den, denl);
}

template<class Sample, class BufferTraits>
template<class CoeffSample> inline
void sos_filter_base<Sample, BufferTraits>::set(const CoeffSample* num, const CoeffSample* den)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<CoeffSample, Sample>));
#endif
	Sample* b = b_;
	Sample* a = a_;
	for (size_t i = 0; i < N_; ++i, b += step_, a += step_, num += section_length, den += section_length) {
		std::copy(num, num + section_length, b);
		std::copy(den, den + section_length, a);
		if (Sample(1) != *a) {
			std::transform(b, b + section_length, b, std::bind2nd(std::divides<Sample>(), *a));
			std::transform(a, a + section_length, a, std::bind2nd(std::divides<Sample>(), *a));
		}
		*a = Sample(); // set a[0] to 0 as an optimization for dot product calculation
		scale_only_[i] = false;
	}
}

template<class Sample, class BufferTraits>
template<class CoeffSample> inline
sos_filter_base<Sample, BufferTraits>::sos_filter_base(size_t N, const CoeffSample* num, const CoeffSample* den)
 :	step_(step())
 , 	N_(N)
 ,	rbuf_(3 * step_ * N_)
 ,	scale_only_(N_)
 ,	b_(rbuf_.get())
 ,	a_(b_ + N_ * step_)
 ,	w_(a_ + N_ * step_)
{
	set(num, den);
}

template<class Sample, class BufferTraits>
inline
sos_filter_base<Sample, BufferTraits>::sos_filter_base(size_t N)
 :	step_(step())
 , 	N_(N)
 ,	rbuf_(3 * step_ * N_)
 ,	scale_only_(N_)
 ,	b_(rbuf_.get())
 ,	a_(b_ + N_ * step_)
 ,	w_(a_ + N_ * step_)
{
}


/*!
 * @brief Implementation of Direct-Form II digital filter realized as a bank of second-order-sections (SOS).
 * @tparam Sample type of samples this filter operates on.
 */
template<class Sample>
class filter_sos: public sos_filter_base<Sample>, public sample_based_transform<Sample>
{
	typedef sos_filter_base<Sample> base;
public:

	using base::section_length;

	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	inline Sample operator()(Sample x) {
		delay(base::w_, base::N_ * base::step_);
		return filter_sample_sos_df2(x, base::N_, base::scale_only_.get(), base::w_, base::b_, base::a_, base::step_);
	}

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
	filter_sos(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl)
	 :	base(N, num, numl, den, denl) {}

	template<class CoeffSample>
	filter_sos(size_t N, const CoeffSample* num, const CoeffSample* den)
	 :	base(N, num, den) {}

	/*!
	 * @brief Prepare filter for operation with given number of sections without actually initializing them.
	 * @param[in] N number of second order sections
	 * @see use set() to actually initialize to coefficient values
	 */
	explicit filter_sos(size_t N): base(N) {}
};

template<>
class DSPXX_API filter_sos<float>: public sos_filter_base<float, dsp::simd::buffer_traits<float> >, public sample_based_transform<float>
{
	typedef sos_filter_base<float, dsp::simd::buffer_traits<float> > base;
public:

	using base::section_length;

	/*!
	 * @brief Apply filtering to a single input sample x.
	 * @param x input sample to filter.
	 * @return filtered sample.
	 */
	float operator()(float x);

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
	filter_sos(size_t N, const CoeffSample (*num)[section_length], const CoeffSize* numl, const CoeffSample (*den)[section_length], const CoeffSize* denl)
	 :	base(N, num, numl, den, denl) {}

	template<class CoeffSample>
	filter_sos(size_t N, const CoeffSample* num, const CoeffSample* den)
	 :	base(N, num, den) {}

	/*!
	 * @brief Prepare filter for operation with given number of sections without actually initializing them.
	 * @param[in] N number of second order sections
	 * @see use set() to actually initialize to coefficient values
	 */
	explicit filter_sos(size_t N): base(N) {}
};

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
	 :	base(b_vec, b_len, L * 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	/*!
	 * @brief Prepare filter for operation with given number of coefficients without actually initializing them.
	 * @param[in] L processing block length
	 * @param[in] b_len number of numerator (MA) coefficients
	 * @param[in] a_len number of denominator (AR) coefficients
	 * @see use set() to actually initialize to coefficient values
	 */
	block_filter(size_t L, size_t b_len, size_t a_len)
	 :	base(a_len, b_len, std::max(b_len, a_len), L * 2)
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
		for (size_t n = 0; n != L_; ++n, --w, ++x) {
			*w = *x;
			*x = filter_sample_df2(w, base::b_, base::M_, base::a_, base::N_);
		}
	}

private:
	const size_t L_;
	Sample* const x_;
};

template<>
class DSPXX_API block_filter<float>: public df2_filter_base<float, dsp::simd::buffer_traits<float> >
{
	typedef df2_filter_base<float, dsp::simd::buffer_traits<float> > base;

public:
	typedef float* iterator;
	typedef const float* const_iterator;

	template<class BIterator, class AIterator>
	block_filter(size_t L, BIterator b_begin, BIterator b_end, AIterator a_begin, AIterator a_end)
	 :	base(b_begin, b_end, a_begin, a_end, L * 2)
	 ,	L_(L)
	 ,	x_(w_ + P_ + L_ - 1) {}

	template<class BIterator>
	block_filter(size_t L, BIterator b_begin, BIterator b_end)
	 :	base(b_begin, b_end, L * 2)
	 ,	L_(L)
	 ,	x_(w_ + P_ + L_ - 1) {}

	template<class BSample, class ASample>
	block_filter(size_t L, const BSample* b_vec, size_t b_len, const ASample* a_vec, size_t a_len)
	 :	base(b_vec, b_len, a_vec, a_len, L * 2)
	 ,	L_(L)
	 ,	x_(w_ + P_ + L_ - 1) {}

	template<class BSample>
	block_filter(size_t L, const BSample* b_vec, size_t b_len)
	 :	base(b_vec, b_len, L * 2)
	 ,	L_(L)
	 ,	x_(w_ + P_ + L_ - 1) {}

	/*!
	 * @brief Prepare filter for operation with given number of coefficients without actually initializing them.
	 * @param[in] L processing block length
	 * @param[in] b_len number of numerator (MA) coefficients
	 * @param[in] a_len number of denominator (AR) coefficients
	 * @see use set() to actually initialize to coefficient values
	 */
	block_filter(size_t L, size_t b_len, size_t a_len)
	 :	base(a_len, b_len, std::max(b_len, a_len), L * 2)
	 ,	L_(L)
	 ,	x_(base::w_ + base::P_ + L_ - 1) {}

	iterator begin() {return x_;}
	iterator end() {return x_ + L_;}
	const_iterator begin() const {return x_ ;}
	const_iterator end() const {return x_ + L_;}

	void operator()();

private:
	const size_t L_;
	float* const x_;
};

}

#endif /* DSP_FILTER_H_INCLUDED */
