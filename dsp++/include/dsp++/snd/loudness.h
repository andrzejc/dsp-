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
#include <numeric>

namespace dsp { namespace snd {

//! @brief K-weighting stage 1 prefilter (high-shelf) gain in dB
#define DSP_SND_K_WEIGHTING_STAGE1_GAIN (4.)
//! @brief K-weighting stage 1 prefilter (high-shelf) quality (Q)
#define DSP_SND_K_WEIGHTING_STAGE1_QUALITY (0.70710678118654752440084436210485)
//! @brief K-weighting stage 1 prefilter (high-shelf) center frequency
#define DSP_SND_K_WEIGHTING_STAGE1_FC (1500.)
//! @brief K-weighting stage 2 prefilter (high-pass) quality (Q)
#define DSP_SND_K_WEIGHTING_STAGE2_QUALITY (0.5)
//! @brief K-weighting stage 2 prefilter (high-pass) cutoff frequency
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

	//! @brief Perform K-weighting filtering
	//! @return filtered sample
	Sample operator()(Sample x) {return flt_(x);}

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
	 * @param[in] wait_full_period if false, start readings immediately after first overlap, otherwise - only after 
	 *	first full frame period is accumulated, which is in accordance with BS.1770 (default)
	 */
	loudness_lkfs(double sr, unsigned channels, double period = 0.4, double overlap = 0.75, const Sample* channel_weights = NULL, bool wait_full_period = true);

	//! @brief Pass next sample for measurement, samples are assumed to be in channel-interleaved format.
	//! Use next_frame() for passing entire audio frame (channel_count() worh of samples) at a time.
	//! @return true if new reading is available (use value() to get it at any time until next reading is available).
	bool operator()(Sample x);
	bool next_sample(Sample x) {return operator()(x);}

	//! @return current loudness measurement (updated each time operator() returns true).
	Sample value() const {return val_;}

	Sample power() const {return dot_;}

	//! @brief Reset the loudness measurement, should be done only at frame boundary (after N*channel count samples).
	void reset(bool wait_full_period = true) {
		std::fill_n(pow_, cc_ * len_, Sample());
		std::fill_n(sum_, cc_, Sample());
		dot_ = val_ = peak_ = Sample();
		i_ = 0;
		first_ = wait_full_period;
	}

	//! @return number of channels the loudness meter is configured for
	unsigned channel_count() const {return cc_;}

	//! @return true if current sample index falls at frame boundary
	bool at_frame_boundary() const {return (0 == (i_ % cc_));}

	//! @brief Pass single frame (channel_count() worth of interleaved samples) for measurement.
	//! @return true if new reading is available (use value() to get it at any time until next reading is available).
	template<class Iterator>
	bool next_frame(Iterator it) {
		bool read = false;
		for (unsigned i = 0; i < cc_; ++i, ++it)
			read |= next_sample(*it);
		return read;
	}

	//! @return peak reading since last reset()
	Sample peak() const {return peak_;}

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
	Sample val_, dot_, peak_;	//!< Current measurement, weighted power sum and peak value
	std::vector<boost::shared_ptr<k_weighting<Sample> > > kw_;
	bool first_;
};

template<class Sample>
loudness_lkfs<Sample>::loudness_lkfs(double sr, unsigned channels, double period, double overlap, const Sample* weights, bool wait_full_period)
	:	sr_(sr)
	,	cc_(channels)
	,	len_(static_cast<unsigned>(period * sr_ + .5))
	,	step_(static_cast<unsigned>((1. - overlap) * period * sr_ + .5) * cc_)
	,	buf_((len_ + 2) * cc_)
	,	pow_(buf_.get())
	,	sum_(pow_ + len_ * cc_)
	,	w_(sum_ + cc_)
{
	reset(wait_full_period);

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
	sum_[c] -= pow_[i_];			
	
	if (sum_[c] < Sample()) // roundoff errors tend to accumulate and power sum may become negative after subtraction
		sum_[c] = Sample();

	sum_[c] += (pow_[i_] = pow(kx, 2) / len_); // calculate moving average of power for next K-weighted sample, ITU-R BS.1770 eq. 1

	++i_;
	i_ %= (len_ * cc_);
	bool reading = (first_ ? (0 == i_) : (0 == (i_ % step_))); // calculate LKFS value only when after full interval
	if (!reading)
		return false;

	first_ = false;
	dot_ = dsp::dot(sum_, w_, cc_);						// weighted sum of channel power
	val_ = Sample(-.691) + Sample(10.) * log10(dot_);	// power in LU, ITU-R BS.1770 eq. 2
	peak_ = std::max(val_, peak_);
	return true;
}

/*!
 * @brief 'EBU Mode' loudness metering according to EBU Tech 3341-2011 and EBU R 128, using 3 meters: M (momentary), S (short-time) and I (integrated).
 * @see EBU Technical Recommendation R 128 ‘Loudness normalisation and permitted maximum level of audio signals’ (https://tech.ebu.ch/docs/r/r128.pdf).
 * @see https://tech.ebu.ch/docs/tech/tech3341.pdf
 */
template<class Sample>
class loudness_ebu {
public:

	/*!
	 * @brief Initialize 'EBU Mode' loudness metering.
	 * @param[in] sr sampling rate
	 * @param[in] channels number of channels
	 * @param[in] channel_weights optional vector of channel scaling weights, defaults to 1.0 for first 3 channels 
	 *	(left, right, center in "standard" layout) and 1.41 for the remaining ones (surround channels).
	 * @param[in] wait_full_period if false, start readings immediately after first overlap, otherwise - only after 
	 *	first full frame period is accumulated, which is in accordance with BS.1770 (default)
	 */
	loudness_ebu(double sr, unsigned channels, const Sample* channel_weights = NULL, bool wait_full_period = true)
		:	m_(sr, channels, 0.4, 0.75, channel_weights, wait_full_period)
		,	s_(sr, channels, 3., 2.9 / 3., channel_weights, wait_full_period)
		,	i_(Sample())
	{
	}


	//! @brief Pass next sample for measurement, samples are assumed to be in channel-interleaved format.
	//! Use next_frame() for passing entire audio frame (channel_count() worh of samples) at a time.
	//! @return true if new reading is available (use value() to get it at any time until next reading is available).
	bool operator()(Sample x) 
	{
		using std::log10;
		s_(x);
		if (!m_(x))
			return false;

		Sample val = m_.value();
		if (val < Sample(-70.))		// if level below -70 LUFS, reading is gated and doesn't count in integrated measurement
			return true;

		g70_.push_back(m_.power());	// store power value for further calculations
		size_t cnt = g70_.size();
		Sample avg = std::accumulate(g70_.begin(), g70_.end(), Sample()) / cnt;	// averaged power across all readings above gating threshold -70 LUFS
		Sample Tr = avg * Sample(0.08529037030705662976325140579496); // 'relative' gating threshold at -10.691 LU, no need to calculate logs and pows, we're operating on power levels here		
		size_t num = 0;
		avg = Sample();
		for (size_t i = 0; i < cnt; ++i) {
			if (g70_[i] > Tr) {
				++num;
				avg += g70_[i];
			}
		}
		avg /= num;					// average of power levels above relative gating threshold
		i_ = Sample(-.691) + Sample(10.) * log10(avg);	// final result in LUFS
		return true;
	}

	bool next_sample(Sample x) {return operator()(x);}

	//! @return number of channels the loudness meter is configured for
	unsigned channel_count() const {return m_.channel_count();}

	//! @return true if current sample index falls at frame boundary
	bool at_frame_boundary() const {return m_.at_frame_boundary();}

	//! @brief Pass single frame (channel_count() worth of interleaved samples) for measurement.
	//! @return true if new reading is available (use value() to get it at any time until next reading is available).
	template<class Iterator>
	bool next_frame(Iterator it) {
		bool read = false;
		for (unsigned i = 0; i < m_.channel_count(); ++i, ++it)
			read |= next_sample(*it);
		return read;
	}

	//! @return current reading of M (momentary) measurement
	Sample value_m() const {return m_.value();}
	//! @return current reading of S (short-time) measurement
	Sample value_s() const {return s_.value();}
	//! @return current reading of I (integrated) measurement
	Sample value_i() const {return i_;}

	//! @brief Reset the loudness measurement, should be done only at frame boundary (after N*channel count samples).
	void reset(bool wait_full_period = true) {
		m_.reset(wait_full_period);
		s_.reset(wait_full_period);
		g70_.clear();
		i_ = Sample();
	}

	//! @brief Read-only access to M (momentary) meter and its properties.
	//! @see meter_s()
	//! @note There's no separate I (integrated) meter, it's just gating applied to readings of M meter.
	const loudness_lkfs<Sample>& meter_m() const {return m_;}

	//! @brief Read-only access to S (short-term) meter and its properties.
	//! @see meter_m()
	//! @note There's no separate I (integrated) meter, it's just gating applied to readings of M meter.
	const loudness_lkfs<Sample>& meter_s() const {return s_;}

private:
	loudness_lkfs<Sample> m_;
	loudness_lkfs<Sample> s_;
	std::vector<Sample> g70_;
	Sample i_;
};

} } 

#endif // DSP_SND_LOUDNESS_H_INCLUDED