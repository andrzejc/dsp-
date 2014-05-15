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

	template<class Sample>
	class k_weighting: public sample_based_transform<Sample> {
	public:

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


	template<class Sample>
	class loudness_lkfs {
	public:

		loudness_lkfs(double sr, unsigned channels, const Sample* weights = NULL);


	private:
		double const sr_;			//!< Sampling rate
		unsigned const cc_;			//!< Channel count
		unsigned const len_;			//!< Length of gating block in samples
		unsigned const step_;		//!< Number of samples after which gating block is advanced
		dsp::trivial_array<Sample> buf_;	
		Sample* const pow_;			
		Sample* const sum_;			//!< Power moving average for each channel
		Sample* const w_;			//!< Channel summing weights
	};

	template<class Sample>
	loudness_lkfs<Sample>::loudness_lkfs(double sr, unsigned channels, const Sample* weights)
	 :	sr_(sr)
	 ,	cc_(channels)
	 ,	len_(static_cast<unsigned>(.4 * sr_ + .5))
	 ,	step_(static_cast<unsigned>(.1 * sr_ + .5))
	 ,	buf_((len_ + 2) * cc_)
	 ,	pow_(buf_.get())
	 ,	sum_(pow_ + len_ * cc_)
	 ,	w_(sum_ + cc_)
	{
		if (NULL != weights) 
			std::copy_n(weights, cc_, w_);
		else {
			std::fill_n(w_, std::min<unsigned>(3, cc_), Sample(1.));
			if (cc_ > 3)
				std::fill_n(w_ + 3, cc_ - 3, Sample(1.41));
		}
	}

} } 

#endif // DSP_SND_LOUDNESS_H_INCLUDED