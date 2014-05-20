/*!
 * @file dsp++/snd/loudness.h
 * @brief Sound loudness estimation routines.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_LOUDNESS_H_INCLUDED
#define DSP_SND_LOUDNESS_H_INCLUDED

#include <dsp++/export.h>
#include <dsp++/algorithm.h>
#include <dsp++/filter.h>
#include <dsp++/vectmath.h>

#include <boost/shared_ptr.hpp>
#include <vector>
#include <algorithm>

namespace dsp { namespace snd {

#define DSP_SND_K_WEIGHTING_STAGE1_GAIN (4.)
#define DSP_SND_K_WEIGHTING_STAGE1_QUALITY (0.70710678118654752440084436210485)
#define DSP_SND_K_WEIGHTING_STAGE1_FC (1500.)
#define DSP_SND_K_WEIGHTING_STAGE2_QUALITY (0.5)
#define DSP_SND_K_WEIGHTING_STAGE2_FC (38.)

	/*! 
	 * @brief Design 2 biquad sections of K-weighting filter for given sampling rate.
	 * @param[in] fs sampling rate.
	 * @param[out] sos_num 2x3 array receiving designed second-order section numerators upon output.
	 * @param[out] sos_den 2x3 array receiving designed second-order section denominator upon output.
	 * @note The filter is designed according to ITU-R BS.1770-3 "Algorithms to measure audio programme loudness and true-peak audio level"
	 * @see http://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-3-201208-I!!PDF-E.pdf
	 */
	DSPXX_API void k_weighting_sos_design(double fs, double sos_num[2][dsp::sos_length], double sos_den[2][dsp::sos_length]);

	/*!
	 * @brief Implementation of two-stage K-weighting prefiltering used in LKFS loudness measure.
	 * @see http://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-3-201208-I!!PDF-E.pdf
	 */
	template<class Sample>
	class k_weighting: public sample_based_transform<Sample> {
	public:

		//! @brief Construct K-weighting filter for given sampling rate.
		//! @param[in] sr sampling rate.
		explicit k_weighting(double sr)
		 :	flt_(2) 
		{
			double num[2][dsp::sos_length];
			double den[2][dsp::sos_length];
			k_weighting_sos_design(sr, num, den);
			flt_.set_coeffs(&num[0][0], &den[0][0]);
		}

		Sample operator()(Sample x) {flt_(x);}

	private:
		dsp::filter_sos<Sample> flt_;

	};

	/*! 
	 * @brief LKFS (aka LUFS) loudness measurement algorithm.
	 * @see http://www.itu.int/dms_pubrec/itu-r/rec/bs/R-REC-BS.1770-3-201208-I!!PDF-E.pdf
	 */
	template<class Sample>
	class loudness_lkfs {
	public:

		/*!
		 * @brief Initialize LKFS measurement algorithm for given sampling rate, number of channels and channel weights.
		 * @param[in] sr sampling rate
		 * @param[in] channels number of channels
		 * @param[in] period gating interval in seconds, default 0.4 (400ms), set to 1.0 to use sample-by-sample sliding window
		 * @param[in] overlap gating interval overlap, default 0.75 (75%)
		 * @param[in] channel_weights optional vector of channel scaling weights, defaults to 1.0 for first 3 channels 
		 *	(left, right, center in "standard" layout) and 1.41 for the remaining ones (surround channels).
		 */
		loudness_lkfs(double sr, unsigned channels, double period = 0.4, double overlap = 0.75, const Sample* channel_weights = NULL);

		//! @return true if new reading is available (use value() to get it at any time).
		bool operator()(Sample x);

		//! @return current loudness measurement (updated each time operator() returns true).
		Sample value() const {return val_;}

		Sample power() const {return dot_;}

	private:
		double const sr_;			//!< Sampling rate
		unsigned const cc_;			//!< Channel count
		unsigned const len_;		//!< Length of gating block in samples
		unsigned const step_;		//!< Number of samples after which gating block is advanced
		unsigned i_;				//!< Current sample index
		dsp::trivial_array<Sample> buf_;	//!< 
		Sample* const pow_;			
		Sample* const sum_;			//!< Power moving average for each channel
		Sample* const w_;			//!< Channel summing weights
		Sample val_, dot_;			//!< Current measurement and weighted power sum
		std::vector<boost::shared_ptr<k_weighting<Sample> > > kw_;
		bool first_;
	};

	template<class Sample>
	loudness_lkfs<Sample>::loudness_lkfs(double sr, unsigned channels, double period, double overlap, const Sample* weights)
	 :	sr_(sr)
	 ,	cc_(channels)
	 ,	len_(static_cast<unsigned>(period * sr_ + .5))
	 ,	step_(static_cast<unsigned>((1. - overlap) * period * sr_ + .5) * cc_)
	 ,	i_(0)
	 ,	buf_((len_ + 2) * cc_)
	 ,	pow_(buf_.get())
	 ,	sum_(pow_ + len_ * cc_)
	 ,	w_(sum_ + cc_)
	 ,	val_(Sample())
	 ,	first_(true)
	{
		std::fill_n(pow_, cc_ * len_, Sample());
		std::fill_n(sum_, cc_, Sample());
		if (NULL != weights) 
			std::copy_n(weights, cc_, w_);
		else {
			std::fill_n(w_, std::min<unsigned>(3, cc_), Sample(1.));
			if (cc_ > 3)
				std::fill_n(w_ + 3, cc_ - 3, Sample(1.41));
		}
		kw_.resize(cc_);
		for (unsigned i = 0; i < cc_; ++i)
			kw_[i].reset(new k_weighting<Sample>(sr_));
	}

	template<class Sample>
	bool loudness_lkfs<Sample>::operator()(Sample x) 
	{
		using std::pow; using std::log10;
		unsigned c = i_ % cc_;
		Sample kx = (*kw_[c])(x);	// k-weighting through appropriate channel prefilter
		sum_[c] -= pow_[i];			
		sum_[c] += pow_[i] = pow(kx, 2) / len_; // calculate moving average of power for next K-weighted sample, ITU-R BS.1770 eq. 1

		++i_;
		i_ %= (len_ * cc_);
		bool reading = (0 == step_) || (first_ ? (0 == i_) : (0 == (i_ % step_))); // calculate LKFS value only when after full interval
		if (!reading)
			return false;

		first_ = false;
		dot_ = dsp::dot(sum_, w_, cc_);
		val_ = Sample(-.691) + Sample(10.) * log10(dot_);  // weighted sum of channel power, ITU-R BS.1770 eq. 2
		return true;
	}

	/*!
	 * @brief 'EBU Mode' loudness metering according to EBU Tech 3341-2011 and EBU R 128.
	 * @see EBU Technical Recommendation R 128 ‘Loudness normalisation and permitted maximum level of audio signals’
	 */
	template<class Sample>
	class loudness_ebu {
	public:

		loudness_ebu(double sr, unsigned channels, const Sample* channel_weights = NULL)
		 :	m_(sr, channels, 0.4, 0.75, channel_weights)
		 ,	s_(sr, channels, 3., 2.9 / 3., channel_weights)
		 ,	i_(Sample())
		{
		}

		bool operator()(Sample x) 
		{
			using std::log10;
			s_(x);
			if (!m_(x))
				return false;

			Sample val = m_.value();
			if (val < Sample(-70.))
				return true;

			g70_.push_back(m_.power());
			size_t cnt = g70_.size();
			Sample avg = std::accumulate(g70_.begin(), g70_.end()) / cnt;
			Sample Tr = avg * Sample(0.08529037030705662976325140579496); // -10.691 LU
			size_t num = 0;
			avg = Sample();
			for (size_t i = 0; i < cnt; ++i) {
				if (g70_[i] > Tr) {
					++num;
					avg += g70_[i];
				}
			}
			avg /= num;
			i_ = Sample(-.691) + Sample(10.) * log10(avg);
		}

		Sample value_m() const {return m_.value();}
		Sample value_s() const {return s_.value();}
		Sample value_i() const {return i_;}

	private:
		loudness_lkfs<Sample> m_;
		loudness_lkfs<Sample> s_;
		std::vector<Sample> g70_;
		Sample i_;
	};

} } 

#endif // DSP_SND_LOUDNESS_H_INCLUDED