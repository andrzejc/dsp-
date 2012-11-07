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

//#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
//#include <boost/concept/requires.hpp>
//#include <boost/concept_check.hpp>
//#endif //DSP_BOOST_CONCEPT_CHECKS_DISABLED

namespace dsp {

/*!
 * @brief Generalized mean calculation.
 */
template<class Sample, class Exponent>
class generalized_mean
{
public:
	/*!
	 * @brief Initialize algorithm for averaging over specified period of samples.
	 * @param L averaging period.
	 * @param p power mean exponent.
	 * @param ic initial condition the preceding samples and "step back" mean value are initialized to.
	 */
	generalized_mean(size_t L, Exponent p, Sample ic = Sample())
	 :	buffer_(L, ((Exponent() == p) ? std::log(ic) : std::pow(ic, p)) / L)
	 ,	pmean_(L * buffer_[0])
	 ,	p_(p)
	 ,	ip_(std::pow(Sample(p), Sample(-1)))
     , 	L_(L)
 	 , 	n_(0)
	{
	}

	Sample operator()(Sample x)
	{
		bool geo = (p_ == Exponent());
		Sample p = (geo ? std::log(x) : std::pow(x, p_)) / L_;
		pmean_ -= buffer_[n_];
		pmean_ += p;
		buffer_[n_] = p;
		++n_;
		n_ %= L_;
		return (geo ? std::exp(pmean_) : std::pow(pmean_, ip_));
	}

private:
	trivial_array<Sample> buffer_;
	Sample pmean_, ip_;
	Exponent p_;
	size_t L_, n_;
};

template<class Sample>
class arithmetic_mean: public generalized_mean<Sample, int>
{
public:
	arithmetic_mean(size_t L, Sample ic = Sample()): generalized_mean<Sample, int>(L, 1, ic) {}
};

template<class Sample>
class geometric_mean: public generalized_mean<Sample, int>
{
public:
	geometric_mean(size_t L, Sample ic = Sample(1)): generalized_mean<Sample, int>(L, 0, ic) {}
};

template<class Sample>
class harmonic_mean: public generalized_mean<Sample, int>
{
public:
	harmonic_mean(size_t L, Sample ic = Sample(1)): generalized_mean<Sample, int>(L, -1, ic) {}
};

template<class Sample>
class quadratic_mean: public generalized_mean<Sample, int>
{
public:
	quadratic_mean(size_t L, Sample ic = Sample()): generalized_mean<Sample, int>(L, 2, ic) {}
};

}

#endif /* DSP_MEAN_H_INCLUDED */
