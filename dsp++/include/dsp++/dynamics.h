/*!
 * @file dsp++/dynamics.h
 * @brief Dynamics (nonlinear) processing algorithms.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_DYNAMICS_H_INCLUDED
#define DSP_DYNAMICS_H_INCLUDED

#include <dsp++/config.h>
#include <dsp++/algorithm.h>
#include <dsp++/mean.h>
#include <dsp++/complex.h>

#include <limits>

namespace dsp {

template<class Sample, class Envelope = dsp::quadratic_mean<Sample> >
class compressor: public dsp::sample_based_transform<Sample> {
public:

	explicit compressor(size_t envelope_L)
	 :	envelope_(envelope_L)
	 ,	threshold_(0)
	 ,	gain_(1.f)
	 ,	ratio_(1.f)
	 ,	attack_delta_(1.)
	 ,	release_delta_(1.)
	 ,	transition_(0)
	{
	}

	float threshold_dB() const {return 20.f * std::log10(threshold_);}
	float threshold() const {return threshold_;}
	void set_threshold_dB(float t) {threshold_ = std::pow(10.f, t/20.f);}
	void set_threshold(float t) {threshold_ = t;}

	float gain_dB() const {return 20.f * std::log10(gain_);}
	float gain() const {return gain_;}
	void set_gain_dB(float g) {gain_ = std::pow(10.f, g/20.f);}
	void set_gain(float g) {gain_ = g;}

	float ratio() const {return ratio_;}
	void set_ratio(float r) {ratio_ = r;}

	void set_attack(size_t sample_count) {attack_delta_ = 1. / sample_count;}
	void set_release(size_t sample_count) {release_delta_ = 1. / sample_count;}

	Sample operator()(Sample x, float* compression_dB = NULL) 
	{
		float ref = static_cast<float>(std::abs(envelope_(x)));			// get signal level from envelope detector
		if (ref > threshold_)
			transition_ = std::min(1., transition_ + attack_delta_);	// adjust transition value according to attack or release time
		else if (ref < threshold_)
			transition_ = std::max(0., transition_ - release_delta_);

		float ratio = 1.f + static_cast<float>(transition_) * (ratio_ - 1.f);	// calculate compression ratio based on current transition value

		float rel = ref / threshold_;			// signal level w/ reference to threshold
		if (ratio != 1.f)
			rel = std::pow(rel, 1.f / ratio);	// scale dB value by ratio (signal level w/reference to threshold after gain is applied)

		float gain = threshold_ * rel / ref;	// calculate actual gain
		if (NULL != compression_dB)
		{
			if (0.f == gain)
				*compression_dB = 20.f * std::log10(std::numeric_limits<float>::epsilon());
			else
				*compression_dB = 20.f * std::log10(gain);
		}

		gain *= gain_;							// apply additional gain set as param
		x = static_cast<Sample>(gain * x);		// amplify the sample
		return x;
	}

private:
	Envelope envelope_;
	float threshold_;
	float gain_;
	float ratio_;
	double attack_delta_;
	double release_delta_;
	double transition_;
};

template<class In> 
struct tanh: public sample_based_transform<In> {
	In operator()(In x) const {using std::tanh; return tanh(x);}
};

template<class Sample, class Functor = dsp::tanh<Sample> >
class limiter: public sample_based_transform<Sample> {
public:

	limiter(const Functor& fun = Functor())
	 :	functor_(fun) 
	{set_threshold_dB(-1);}

	Sample threshold_dB() const {using std::log10; return Sample(20) * log10(threshold_);}
	Sample threshold() const {return threshold_;}
	void set_threshold_dB(Sample t) {using std::pow; set_threshold(pow(Sample(10), t/20));}
	void set_threshold(Sample t) {threshold_ = t; swing_ = 1 - threshold_;}

	Sample operator()(Sample x) 
	{
		using std::abs; // allow for ADL with non-std abs
		Sample a = static_cast<Sample>(abs(x));
		if (a <= threshold_)
			return x;
		Sample in = (a - threshold_)/swing_;
		Sample out = functor_(in);
		Sample gain = (threshold_ + out * swing_) / a;
		return x * gain;
	}

private:
	Functor functor_;
	Sample threshold_;
	Sample swing_;
	Sample limit_;
};

}

#endif /* DSP_DYNAMICS_H_INCLUDED */
