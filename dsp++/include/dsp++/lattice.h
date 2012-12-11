/*!
 * @file dsp++/lattice.h
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_LATTICE_H_INCLUDED
#define DSP_LATTICE_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/trivial_array.h>
#include <dsp++/utility.h>
#include <dsp++/algorithm.h>

#include <algorithm>
#include <functional>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif

namespace dsp {

/*!
 * @brief Implementation of FIR (all-zero) Lattice filter structure.
 * This filter structure realizes direct linear predictor.
 * @see http://www.cs.tut.fi/~tabus/course/ASP/LectureNew8.pdf
 */
template<class Sample>
class lattice_fir: public sample_based_transform<Sample>
{
public:
	/*!
	 * @brief Filter single input sample x, return single output sample (forward prediction error)
	 * and optionally backward prediction error.
	 * @param[in]  x input sample
	 * @param[out]  eb pointer to optional backward prediction error result
	 * @return output sample (forward prediction error)
	 */
	Sample operator()(Sample x, Sample* eb = NULL);

	/*!
	 * @brief Construct lattice FIR filter with reflection coefficients provided as a sequence
	 * of iterators [k_begin, k_end).
	 */
	template<class Iterator>
	lattice_fir(Iterator k_begin, Iterator k_end);

	/*!
	 * @brief Construct lattice FIR filter with reflection coefficients provided as a vector.
	 */
	template<class KSample>
	lattice_fir(const KSample* k_vec, size_t k_len);

	/*!
	 * @return order of the filter
	 */
	size_t order() const {return M_;}
private:
	const size_t M_;			//!< Number of MA coefficients.
	trivial_array<Sample> buffer_;
	Sample* const k_;				//!< reflection coefficients (M_)
	Sample* const b_;				//!< backward prediction error delay line (M_ + 1)
};

template<class Sample>
Sample lattice_fir<Sample>::operator()(Sample x, Sample* eb)
{
	Sample* b = b_;
	Sample* k = k_;
	*b = x;			// initialize b[0] to f[0]
	++b;			// move to b[1] before entering the loop
	for (size_t i = 0; i < M_; ++i, ++b, ++k)
	{
		Sample fn = x + *k * *b;	// use temporary value for f[i + 1], f[i] will be needed for b[i]
		*b += *k * x;			// calculate b[i] using value of f[i]
		x = fn;					// move f to next index
	}

	if (NULL != eb)
		*eb = *(b - 1);	// b[N] has backward prediction error sample
	delay(b_, M_ + 1);	// single-step delay line
	return x;
}

template<class Sample>
template<class Iterator>
lattice_fir<Sample>::lattice_fir(Iterator k_begin, Iterator k_end)
 :	M_(std::distance(k_begin, k_end))
 ,	buffer_(2 * M_ + 1)
 ,	k_(buffer_.get())
 ,	b_(k_ + M_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<Iterator, Sample>));
#endif

	std::copy(k_begin, k_end, k_);
}

template<class Sample>
template<class KSample>
lattice_fir<Sample>::lattice_fir(const KSample* k_vec, size_t k_len)
 :	M_(k_len)
 ,	buffer_(2 * M_ + 1)
 ,	k_(buffer_.get())
 ,	b_(k_ + M_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<KSample, Sample>));
#endif

	std::copy(k_vec, k_vec + k_len, k_);
}

/*!
 * @brief Implementation of IIR (all-pole) Lattice filter structure.
 * This is almost identical to dsp::lattice_ladder, but with the ladder (FIR)
 * part omitted.
 * @see http://web.eecs.utk.edu/~roberts/ECE506/PresentationSlides/LatticeLadder.pdf
 */
template<class Sample>
class lattice_iir: public sample_based_transform<Sample>
{
public:
	Sample operator()(Sample x, Sample* eb = NULL);

	template<class Iterator>
	lattice_iir(Iterator k_begin, Iterator k_end);

	template<class KSample>
	lattice_iir(const KSample* k_vec, size_t k_len);

	size_t order() const {return N_ - 1;}

private:
	const size_t N_;			//!< Number of AR coefficients.
	trivial_array<Sample> buffer_;
	Sample* const k_;				//!< reflection coefficients (N_)
	Sample* const g_;				//!< backward prediction error delay line (M_ + 1)
};

template<class Sample>
Sample lattice_iir<Sample>::operator()(Sample x, Sample* eb)
{
	Sample* k = k_ + N_- 1;
	Sample* g = g_ + N_;
	for (size_t i = 0; i < N_; ++i, --g, --k)
	{
		x -= (*g) * (*k);
		(*g) += x * (*k);
	}
	*g = x;
	if (NULL != eb)
		*eb = *(g_ + N_);
	delay(g_, N_ + 1);
	return x;
}

template<class Sample>
template<class Iterator>
lattice_iir<Sample>::lattice_iir(Iterator k_begin, Iterator k_end)
 :	N_(std::distance(k_begin, k_end))
 ,	buffer_(2 * N_ + 1)
 ,	k_(buffer_.get())
 ,	g_(k_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<Iterator, Sample>));
#endif

	std::copy(k_begin, k_end, k_);
}

template<class Sample>
template<class KSample>
lattice_iir<Sample>::lattice_iir(const KSample* k_vec, size_t k_len)
 :	N_(k_len)
 ,	buffer_(2 * N_ + 1)
 ,	k_(buffer_.get())
 ,	g_(k_ + N_)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<KSample, Sample>));
#endif

	std::copy(k_vec, k_vec + k_len, k_);
}

/*!
 * @brief Implementation of mixed FIR-IIR Lattice-Ladder filter structure.
 * @see http://web.eecs.utk.edu/~roberts/ECE506/PresentationSlides/LatticeLadder.pdf
 */
template<class Sample>
class lattice_ladder: public sample_based_transform<Sample>
{
public:
	Sample operator()(Sample x, Sample* eb = NULL);

	template<class KIterator, class VIterator>
	lattice_ladder(KIterator k_begin, KIterator k_end, VIterator v_begin, VIterator v_end);

	template<class KSample, class VSample>
	lattice_ladder(const KSample* k_vec, size_t k_len, const VSample* v_vec, size_t v_len);

	size_t order() const {return N_ - 1;}

private:
	const size_t N_;			//!< Number of AR/MA coefficients.
	trivial_array<Sample> buffer_;
	Sample* const k_;				//!< reflection (lattice) coefficients (N_)
	Sample* const v_;				//!< ladder coefficients (N_ + 1)
	Sample* const g_;				//!< one-sample delayed backward prediction error (N_ + 1)
};

template<class Sample>
Sample lattice_ladder<Sample>::operator()(Sample x, Sample* eb)
{
	Sample* k = k_ + N_- 1;
	Sample* v = v_ + N_;
	Sample* g = g_ + N_;
	Sample r = Sample();
	for (size_t i = 0; i < N_; ++i, --g, --k, --v)
	{
		x -= (*g) * (*k);
		(*g) += x * (*k);
		r += (*g) * (*v);
	}
	*g = x;
	r += x * (*v);
	if (NULL != eb)
		*eb = *(g_ + N_);
	delay(g_, N_ + 1);
	return r;
}

template<class Sample>
template<class KIterator, class VIterator>
lattice_ladder<Sample>::lattice_ladder(KIterator k_begin, KIterator k_end, VIterator v_begin, VIterator v_end)
 :	N_(std::max(std::distance(k_begin, k_end), std::distance(v_begin, v_end)))
 ,	buffer_(3 * N_ + 2)
 ,	k_(buffer_.get())
 ,	v_(k_ + N_)
 ,	g_(v_ + N_ + 1)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<KIterator, Sample>));
	BOOST_CONCEPT_ASSERT((boost::OutputIterator<VIterator, Sample>));
#endif

	std::copy(k_begin, k_end, k_);
	std::copy(v_begin, v_end, v_);
}

template<class Sample>
template<class KSample, class VSample>
lattice_ladder<Sample>::lattice_ladder(const KSample* k_vec, size_t k_len, const VSample* v_vec, size_t v_len)
 :	N_(std::max(k_len, v_len))
 ,	buffer_(3 * N_ + 2)
 ,	k_(buffer_.get())
 ,	v_(k_ + N_)
 ,	g_(v_ + N_ + 1)
{
#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
	BOOST_CONCEPT_ASSERT((boost::Convertible<KSample, Sample>));
	BOOST_CONCEPT_ASSERT((boost::Convertible<VSample, Sample>));
#endif

	std::copy(k_vec, k_vec + k_len, k_);
	std::copy(v_vec, v_vec + v_len, v_);
}

}

#endif /* DSP_LATTICE_H_INCLUDED */
