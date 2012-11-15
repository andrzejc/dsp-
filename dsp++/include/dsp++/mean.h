/*!
 * @file dsp++/mean.h
 * @brief Algorithms for calculating mean values.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_MEAN_H_INCLUDED
#define DSP_MEAN_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/trivial_array.h>
#include <dsp++/algorithm.h>

#include <cmath>
#include <stdexcept>

namespace dsp {

/*!
 * @brief Generalized mean calculation for running series of values (e.g. online algorithms).
 * Generalized mean of order p is @f$M_p(x_1,\dots,x_n) = \left( \frac{1}{n} \sum_{i=1}^n x_i^p \right)^{1/p}@f$.
 * @see http://en.wikipedia.org/wiki/Generalized_mean
 * @tparam Sample type of sample this algorithm works with (real and complex types).
 * @tparam Exponent type of the exponent value, there may be some optimizations possible when it's integer.
 */
template<class Sample, class Exponent>
class generalized_mean: public sample_based_transform<Sample>
{
public:
	/*!
	 * @brief Initialize algorithm for averaging over specified period of samples.
	 * @param L averaging period.
	 * @param p mean exponent; notable values: 0 - geometric mean, -1 - harmonic mean, 2 - quadratic mean (RMS).
	 * @param ic initial condition the preceding samples and "step back" mean value are initialized to. Important
	 * when Sample is a real type and working with non-greater-than-zero values.
	 */
	generalized_mean(size_t L, Exponent p, Sample ic = Sample())
	 :	buffer_(L, ((Exponent() == p) ? std::log(ic) : std::pow(ic, p)) / L)
	 ,	pmean_(L * buffer_[0])
	 ,	ip_(std::pow(Sample(p), Sample(-1)))
	 ,	p_(p)
     , 	L_(L)
 	 , 	n_(0)
	{
	}

	/*!
	 * @brief Move the averaging window to next sample and calculate the mean value.
	 * @param x next (subsequent) sample.
	 * @return mean value of x and (L - 1) previous samples.
	 */
	Sample operator()(Sample x)
	{
		bool geo = (p_ == Exponent());
		Sample p = (geo ? std::log(x) : std::pow(x, p_)) / L_; 	// in case of geometric mean use logs as intermediate values, otherwise - powers
		pmean_ -= buffer_[n_];	// subtract oldest intermediate value from previous step result
		pmean_ += p;			// add current intermediate value
		buffer_[n_] = p;		// and store it in circular buffer so that it can be subtracted when we advance by L_ samples
		++n_;					// move circular buffer to next index
		n_ %= L_;
		return (geo ? std::exp(pmean_) : std::pow(pmean_, ip_)); // return the appropriate root of the intermediate sum
	}

private:
	trivial_array<Sample> buffer_;	//!< L_-length (circular) buffer holding intermediate values (averaged powers or logs if p_ == 0)
	Sample pmean_;					//!< previous step mean value
	Sample ip_;						//!< inverted exponent (can't be the same type as p_, as it often will be integer, and we need it for root calculation)
	Exponent p_;					//!< mean exponent
	size_t L_;						//!< averaging period and buffer_ length
	size_t n_;						//!< index of current sample in the circular buffer
};

/*!
 * @brief A special case: generalized mean of order 1.
 */
template<class Sample>
class arithmetic_mean: public generalized_mean<Sample, int>
{
public:
	arithmetic_mean(size_t L, Sample ic = Sample()): generalized_mean<Sample, int>(L, 1, ic) {}
};

/*!
 * @brief A special case: generalized mean of order 0, @f$M_0(x_1,\dots,x_n) = \prod_{i=1}^n x_i^{w_i}@f$
 *  (actually geometric mean is @f$\lim_{p\to0} M_p(x_1,\dots,x_n)@f$).
 */
template<class Sample>
class geometric_mean: public generalized_mean<Sample, int>
{
public:
	geometric_mean(size_t L, Sample ic = Sample(1)): generalized_mean<Sample, int>(L, 0, ic) {}
};

/*!
 * @brief A special case: generalized mean of order -1, @f$M_{-1}(x_1,\dots,x_n) = \frac{n}{\frac{1}{x_1}+\dots+\frac{1}{x_n}}@f$.
 */
template<class Sample>
class harmonic_mean: public generalized_mean<Sample, int>
{
public:
	harmonic_mean(size_t L, Sample ic = Sample(1)): generalized_mean<Sample, int>(L, -1, ic) {}
};

/*!
 * @brief A special case: generalized mean of order 2, RMS value.
 */
template<class Sample>
class quadratic_mean: public generalized_mean<Sample, int>
{
public:
	quadratic_mean(size_t L, Sample ic = Sample()): generalized_mean<Sample, int>(L, 2, ic) {}
};

}

#endif /* DSP_MEAN_H_INCLUDED */
