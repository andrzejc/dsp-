/*!
 * @file dsp++/rt/multiband_compressor.h
 * @brief Multiband compressor component.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_RT_MULTIBAND_COMPRESSOR_H_INCLUDED
#define DSP_RT_MULTIBAND_COMPRESSOR_H_INCLUDED

#include <boost/smart_ptr.hpp>
#include <vector>

namespace dsp { namespace snd { class format; }}

namespace dsp { namespace rt {

namespace detail { struct mbc_impl;}

class multiband_compressor {
public:

	typedef float sample_type; //!< This component uses float type as its sample representation

	//! @brief Parameters of a single band.
	struct band_params 
	{
		float envelope_period_ms;	//!< Averaging period for envelope calculations. This is set per-band, as high-frequency 
									//!< bands may use much shorter period. Once set for any band, this can't be changed.
		float threshold_dB;			//!< Compression threshold in dB.
		float gain_dB;				//!< Gain applied to this band in dB.
		float ratio;				//!< Compression ratio.
		float attack_ms;			//!< Attack time in ms (time it takes for the compressor to fully activate after the threshold_dB is exceeded).
		float release_ms;			//!< Release time in ms (time it takes for the compressor to deactivate).
		bool bypass;				//!< If set, the compressor in this band is disabled.
		bool mute;					//!< If set, this band is entirely muted.
	};

	//! @brief Parameters of the whole component.
	struct params 
	{
		unsigned sample_rate;	//!< The sampling frequency this component uses for designing filters & other calculations.
		unsigned channel_count; //!< Number of channels in a sample frame. Samples in a processing block are assumed to be interleaved (channel-first order).
		unsigned block_size;	//!< Number of sample frames in a processing block (the total number of samples is block_size * channel_count).
		std::vector<float> crossover_frequencies;	//!< Frequencies dividing the signal into bands (bands.size() - 1).
		std::vector<band_params> bands;				//!< Parameters of compressors in each of the bands.
		float limiter_threshold_dB;	//!< Threshold above which tanh-shaped limiter is activated, if limiter_bypass is not set.
		bool limiter_bypass;		//!< Controls whether limiter is enabled.
	};

	multiband_compressor(unsigned sample_rate, unsigned channel_count, unsigned block_size);
	explicit multiband_compressor(const params& p);

	~multiband_compressor();

	void fill_params(params& p) const;
	void set_params(const params& p);

	unsigned sample_rate() const;
	unsigned block_size() const;
	unsigned channel_count() const;
	
	void fill_format(dsp::snd::format& f) const;

	void set_crossover_frequency(unsigned index, float frequency);
	float crossover_frequency(unsigned index) const;

	unsigned band_count() const;

	void fill_band_params(unsigned index, band_params& p) const;
	void set_band_params(unsigned index, const band_params& p);

	float limiter_threshold_dB() const;
	void set_limiter_threshold_dB(float t);

	bool is_limiter_bypass() const;
	void set_limiter_bypass(bool b);

private:
	boost::scoped_ptr<detail::mbc_impl> impl_;
};

} }

#endif // DSP_MULTIBAND_COMPRESSOR_H_INCLUDED