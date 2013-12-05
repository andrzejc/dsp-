/*!
 * @file dsp++/levinson.h
 * @brief Algorithms for Levinson-Durbin recursion.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_LEVINSON_H_INCLUDED
#define DSP_LEVINSON_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/complex.h>
#include <dsp++/trivial_array.h>

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <cmath>
#include <iterator>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif

namespace dsp {

/*!
 * @brief Calculate P'th order prediction polynomial Acur based on P+1'th order prediction polynomial, Anxt.
 * @param[in] P order of the prediction polynomial to calculate.
 * @param[in] Anxt sequence of coefficients of P+1'th order prediction polynomial (of length P+2).
 * @param[out] Acur sequence of coefficients of P'th order prediction polynomial (of length P+1);
 * @param[out] Kcur P+1'th order reflection coefficient.
 * @param[in] Enxt P+1'th order prediction error used for calculation of Ecur (set both to NULL if not needed).
 * @param[out] Ecur P'th order prediction error, based on given P+1'th order prediction error Enxt.
 * @throw std::domain_error if last (P+1'th) element of input sequence Anxt is equal 1.
 * @tparam ANxtIter type of iterator used to pass input sequence Anxt (bidirectional iterator).
 * @tparam ACurIter type of iterator used to pass output sequence Acur (output iterator).
 */
template<class ANxtIter, class ACurIter>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<ANxtIter>))
		((boost::OutputIterator<ACurIter, typename std::iterator_traits<ANxtIter>::value_type>)),
		(void)) inline
#else
		void inline
#endif
levdown(size_t P, ANxtIter Anxt, ACurIter Acur,
		typename std::iterator_traits<ANxtIter>::value_type* Kcur = NULL,
		const typename std::iterator_traits<ANxtIter>::value_type* Enxt = NULL,
		typename std::iterator_traits<ANxtIter>::value_type* Ecur = NULL)
{
	typedef typename std::iterator_traits<ANxtIter>::value_type Sample;
	typedef typename dsp::remove_complex<Sample>::type Real;
	ANxtIter rAnxt = Anxt;
	std::advance(rAnxt, P + 1);
	const Sample knxt = *rAnxt; --rAnxt;
	const Real mknxt = std::abs(knxt);
	if (Real(1) == mknxt)
		throw std::domain_error("dsp::levdown() reflection coefficient equal 1.0");

	*Acur = Sample(1); ++Acur;
	++Anxt;
	const Real iden = Real(1)/(Real(1) - std::pow(mknxt, 2)); // inverse of denominator 1/(1 - abs(knxt)^2)
	for (size_t i = 0; i < P; ++i, ++Anxt, ++Acur, --rAnxt)
		*Acur = (*Anxt - knxt * dsp::conj(*rAnxt)) * iden;

	if (NULL != Enxt && NULL != Ecur)
		*Ecur = *Enxt * iden;
	if (NULL != Kcur)
		*Kcur = knxt;
}

/*!
 * @brief Calculate P+1'th order prediction polynomial Anxt based on P'th order prediction polynomial, Acur.
 * @param[in] P order of the prediction polynomial to base calculations on.
 * @param[in] Acur sequence of coefficients of P'th order prediction polynomial (of length P+1).
 * @param[out] Anxt sequence of coefficients of P+1'th order prediction polynomial (of length P+2);
 * @param[out] Knxt P+1'th order reflection coefficient.
 * @param[in] Ecur P'th order prediction error.
 * @param[out] Enxt P+1'th order prediction error based on Enxt (set both to NULL if not needed).
 * @tparam ANxtIter type of iterator used to pass input sequence Anxt (bidirectional iterator).
 * @tparam ACurIter type of iterator used to pass output sequence Acur (output iterator).
 */
template<class ACurIter, class ANxtIter>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<ACurIter>))
		((boost::OutputIterator<ANxtIter, typename std::iterator_traits<ACurIter>::value_type>)),
		(void)) inline
#else
		void inline
#endif
levup(size_t P, ACurIter Acur, ANxtIter Anxt,
		typename std::iterator_traits<ACurIter>::value_type Knxt,
		const typename std::iterator_traits<ACurIter>::value_type* Ecur = NULL,
		typename std::iterator_traits<ACurIter>::value_type* Enxt = NULL)
{
	typedef typename std::iterator_traits<ACurIter>::value_type Sample;
	typedef typename dsp::remove_complex<Sample>::type Real;
	ACurIter rAcur = Acur;
	std::advance(rAcur, P);
	++Acur;
	*Anxt = Sample(1); ++Anxt;
	for (size_t i = 0; i < P; ++i, ++Anxt, ++Acur, --rAcur)
		*Anxt = *Acur + Knxt * dsp::conj(*rAcur);
	*Anxt = Knxt;

	if (NULL != Enxt && NULL != Ecur)
		*Enxt = *Ecur * (Real(1) - std::pow(std::abs(Knxt), 2));
}

/*!
 * @brief Levinson-Durbin recursion functor operating on generic sample types.
 * @see http://en.wikipedia.org/wiki/Levinson_recursion
 */
template<class Sample, class Allocator = std::allocator<Sample> >
class levinson
{
public:
	//! @return the order of the recursion N (and the number of calculated reflection coefficients k).
	size_t recursion_order() const {return N_;}
	//! @return length of input autocorrelation sequence.
	size_t input_length() const {return L_;}

	/*!
	 * @brief Perform Levinson-Durbin recursion on input autocorrelation sequence (of length input_length())
	 * starting at r_begin and place the solution (polynomial coefficients) into sequence starting at a_begin
	 * (of length recursion_order() + 1).
	 * @param[in] r_begin start of input autocorrelation sequence.
	 * @param[out] a_begin start of output polynomial coeficients sequence.
	 * @return final prediction error.
	 * @tparam RIterator type of iterator used to specify input sequence.
	 * @tparam AIterator type of iterator used to receive output sequence. Its value type must be assignable
	 * from Sample.
	 */
	template<class RIterator, class AIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<RIterator>))
			((boost::OutputIterator<AIterator, typename std::iterator_traits<RIterator>::value_type>)),
			(Sample))
#else
			Sample
#endif
	operator()(RIterator r_begin, AIterator a_begin)
	{return do_calc(r_begin, a_begin, static_cast<AIterator*>(NULL));}

	/*!
	 * @copydoc operator()(RIterator,AIterator)
	 * @param[out] k_begin start of output reflection coefficients sequence of length recursion_order().
	 * @tparam KIterator type of iterator used to receive reflection coefficients sequence. Its value type must
	 * be assignable from Sample.
	 */
	template<class RIterator, class AIterator, class KIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<RIterator>))
			((boost::OutputIterator<AIterator, typename std::iterator_traits<RIterator>::value_type>))
			((boost::OutputIterator<KIterator, typename std::iterator_traits<RIterator>::value_type>)),
			(Sample))
#else
			Sample
#endif
	operator()(RIterator r_begin, AIterator a_begin, KIterator k_begin)
	{return do_calc(r_begin, a_begin, &k_begin);}

	/*!
	 * @brief Initialize Levinson-Durbin functor to perform N'th order recursion on the input autocorrelation
	 * sequence of length r_len.
	 * @param r_len length of input autocorrelation sequence.
	 * @param N recursion order (must be less than r_len, if set to 0, r_len - 1 is assumed).
	 * The length of output polynomial coefficients sequence a will be N + 1, and the number of output reflection
	 * coefficients k will be N.
	 * @throw std::domain_error if (N >= r_len).
	 */
	explicit levinson(size_t r_len, size_t N = 0);

private:
	size_t L_;						//!< length of input autocorrelation sequence r
	size_t N_;						//!< recursion order
	trivial_array<Sample, Allocator> a_;

	template<class RIterator, class AIterator, class KIterator>
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_REQUIRES(((boost::BidirectionalIterator<RIterator>))
			((boost::OutputIterator<AIterator, typename std::iterator_traits<RIterator>::value_type>))
			((boost::OutputIterator<KIterator, typename std::iterator_traits<RIterator>::value_type>)),
			(Sample))
#else
			Sample
#endif
	do_calc(RIterator r_begin, AIterator a_begin, KIterator* k_begin)
	{
		const Sample zero = Sample();
		const size_t size = N_ + 1;
		Sample em = *r_begin;
		if (zero == em)
		{
			std::fill_n(a_begin, size, zero);
			if (NULL != k_begin)
				std::fill_n(*k_begin, N_, zero);
			return zero;
		}

		Sample* acur = a_.get();
		Sample* anxt = acur + size;
		*acur = Sample(1); ++acur;
		std::fill_n(acur, N_, zero);

		RIterator r0 = r_begin;
		for (size_t m = 1; m < size; ++m, ++r0)                  //m=2:N+1
		{
			Sample km = zero;                    	//err = 0;
			RIterator r = r0;
			RIterator rm = r0; ++rm;
			for (size_t k = 1; k < m; ++k, ++acur, --r)   //for k=2:m-1
				km -= (*acur) * (*r);        			// err = err + am1(k)*R(m-k+1);
			(km -= *rm) /= em;

			if (NULL != k_begin)
			{
				**k_begin = km;
				++(*k_begin);
			}

			acur = a_.get();
			levup(m - 1, acur, anxt, km, &em, &em);
			if (zero == em)
				break;

			std::copy(anxt, anxt + size, acur);			//for s=1:N+1 // am1(s) = acur(s)
			++acur;
		}
		std::copy(anxt, anxt + size, a_begin);
		return em;
	}

	size_t verify_order(size_t N)
	{
		if (0 == N)
			return L_ - 1;
		if (N >= L_)
			throw std::domain_error("dsp::levinson recursion order must be less than autocorrelation sequence length");
		return N;
	}

	static size_t verify_input_length(size_t L)
	{
		if (0 == L)
			throw std::domain_error("dsp::levinson input autocorrelation sequence must not be empty");
		return L;
	}
};

template<class Real>
levinson<Real>::levinson(size_t r_len, size_t N)
 :	L_(verify_input_length(r_len))
 ,	N_(verify_order(N))
 ,	a_(2 * N_ + 2)
{}

}

#endif /* DSP_LEVINSON_H_INCLUDED */
