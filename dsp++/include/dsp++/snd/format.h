/*!
 * @file dsp++/snd/format.h
 * @brief Sound (audio) format description.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_FORMAT_H_INCLUDED
#define DSP_SND_FORMAT_H_INCLUDED

#include <dsp++/export.h>

#include <cstddef>
#include <string>
#include <bitset>

namespace dsp { namespace snd {

//! Constants, functions, etc. related to labeling channels in a multichannel sound file formats.
namespace channel {

//! @brief Channel type indexes (related to the speaker location). 
//! This is ordered according to USB Audio class definition, which is a native representation both in WAVE files and in CoreAudio.
//! @see http://www.usb.org/developers/devclass_docs/audio10.pdf
//! @see http://msdn.microsoft.com/en-us/windows/hardware/gg463006
//! @see https://developer.apple.com/library/mac/#qa/qa1638/_index.html
namespace location { enum label {
	front_left,
	front_right,
	front_center,
	lfe,
	back_left,
	back_right,
	front_left_center,
	front_right_center,
	back_center,
	side_left,
	side_right,
	top_center,
	top_front_left,
	top_front_center,
	top_front_right,
	top_back_left,
	top_back_center,
	top_back_right,

	count_						//!< Number of labels defined so far.
}; } // namespace type


//! @brief Channel speaker assignment masks.
//! @see refer to http://en.wikipedia.org/wiki/Surround_sound for standard speaker channel assignment
namespace mask { enum label {
	unknown = 0,
#define DSP_SND_CHANNEL_MASK(label) label = 1 << (location:: label)
	DSP_SND_CHANNEL_MASK(front_left),
	DSP_SND_CHANNEL_MASK(front_right),
	DSP_SND_CHANNEL_MASK(front_center),
	DSP_SND_CHANNEL_MASK(lfe),
	DSP_SND_CHANNEL_MASK(back_left),
	DSP_SND_CHANNEL_MASK(back_right),
	DSP_SND_CHANNEL_MASK(front_left_center),
	DSP_SND_CHANNEL_MASK(front_right_center),
	DSP_SND_CHANNEL_MASK(back_center),
	DSP_SND_CHANNEL_MASK(side_left),
	DSP_SND_CHANNEL_MASK(side_right),
	DSP_SND_CHANNEL_MASK(top_center),
	DSP_SND_CHANNEL_MASK(top_front_left),
	DSP_SND_CHANNEL_MASK(top_front_center),
	DSP_SND_CHANNEL_MASK(top_front_right),
	DSP_SND_CHANNEL_MASK(top_back_left),
	DSP_SND_CHANNEL_MASK(top_back_center),
	DSP_SND_CHANNEL_MASK(top_back_right),
#undef DSP_SND_CHANNEL_MASK
}; } // namespace mask

//! Channel speaker configuration mask combinations.
namespace config { enum label {
	mono =			mask::front_center,
	stereo =		mask::front_left | mask::front_right,
	s2_1 =			stereo | mask::lfe,
	s3_0_stereo =	stereo | mask::front_center,
	s3_0_surround =	stereo | mask::back_center,
	s4_0_quadro =	stereo | mask::back_left | mask::back_right,
	s4_0_surround =	s3_0_stereo | mask::back_center,
	s5_0	=		s3_0_stereo | mask::back_left | mask::back_right,
	s5_0_side =		s3_0_stereo | mask::side_left | mask::side_right,
	s5_1 =			s5_0 | mask::lfe,
	s5_1_side =		s5_0_side | mask::lfe,
	s6_0 =			s5_0 | mask::back_center,
	s6_0_side =		s5_0_side | mask::back_center,
	s7_0 =			s5_0 | mask::side_left | mask::side_right,
	s7_1 =			s7_0 | mask::lfe,
}; 

//! @param[in] channel_count number of channels.
//! @return "default" (most typical) speaker configuration mask for given channel count.
DSPXX_API unsigned default_for(unsigned channel_count);

} // namespace config

//! @brief Bitset for storing channel speaker layouts.
typedef std::bitset<location::count_> layout;

//! @brief Used to denote that given channel type is missing in the layout.
//! @see dsp::snd::format::channel_index()
const unsigned not_present = unsigned(-1);
}

namespace sample {

//! @brief Labels of audio sample formats (these are used primarily for interfacing with @p dsp::snd::reader and @p dsp::snd::writer, or hardware I/O).
namespace label {
const char* const u8 =	"U8";	//!< Unsigned 8-bit integer with offset of 128 linear PCM.
const char* const s8 =	"S8"; 	//!< Signed 8-bit integer, linear PCM.
const char* const s16 = "S16";	//!< Signed 16-bit integer linear PCM.
const char* const s24 = "S24";	//!< Signed 24-bit integer (packed) linear PCM.
const char* const s32 = "S32";	//!< Signed 32-bit integer linear PCM.
const char* const f32 = "F32";	//!< Floating-point 32-bit (with a non-overdriving range of [-1.0, 1.0]).
const char* const f64 = "F64";	//!< Floating-point 64-bit (with a non-overdriving range of [-1.0, 1.0]).
}

//! @brief Used to denote that bit size of sample is unknown in the given format.
//! @see dsp::snd::sample::bit_size_of()
const unsigned size_unknown = 0;

//! @param[in] format_label format label to parse for sample bit size
//! @return bit size of sample in given format, or @p size_unknown if malformed label.
DSPXX_API unsigned bit_size_of(const char* format_label);

//! @brief Sample type labels as returned by dsp::snd::sample::type_of().
namespace type { enum label {
	unknown,
	pcm_unsigned,
	pcm_signed,
	ieee_float,
}; } // namespace type

//! @param[in] format_label format label to parse for sample type.
//! @return sample type of given format.
DSPXX_API type::label type_of(const char* format_label);
}

namespace file_type {

namespace label {
const char* const wav =			"wav";
const char* const aiff =		"aiff";
const char* const au =			"au";
const char* const raw =			"raw";
const char* const wav64 =		"wav64";
const char* const matlab4 =		"mat4";
const char* const matlab5 =		"mat5";
const char* const flac =		"flac";
const char* const core_audio =	"caf";
const char* const ogg =			"ogg";
}

/// @return file type id (@p file_type::label constant) for specified file extension, or NULL if unknown.
DSPXX_API const char* const for_extension(const char* ext);
/// @return file extension which should be used with specified file type id (@p file_type::label), or NULL if unknown.
DSPXX_API const char* const extension_for(const char* cnt);
/// @return file type id (@p file_type::label constant) for specified MIME subtype (assuming MIME type is audio/<i>subtype</i>), or NULL if unknown.
DSPXX_API const char* const for_mime_subtype(const char* st);
/// @return MIME subtype which should be used with specified file type id (@p file_type::label), or NULL if unknown.
DSPXX_API const char* const mime_subtype_for(const char* cnt);

}

const unsigned sampling_rate_audio_cd = 44100;
const unsigned sampling_rate_phone_narrow = 8000;
const unsigned sampling_rate_phone_wide = 16000;
const unsigned sampling_rate_dat_lp = 32000;
const unsigned sampling_rate_dat = 48000;

enum format_tag_ {
	format_channel_mask
};

/// @brief Describes audio stream settings, namely: number (and optionally layout) of channels, sampling rate and sample format.
/// Note that the sample format adheres primarily to the format which is used internally by I/O classes, i.e. the hardware or I/O format,
/// not the sample abstraction used by @p dsp::snd APIs (which tend to use float or double types, mapping to @p sample::label::f32 &amp; @p sample::label::f64).
class DSPXX_API format
{
public:
	/// @return Channel layout in the form of bitset with specific @p channel::location flags set.
	const channel::layout& channel_layout() const {return channel_layout_;}

	/// @brief Set channel layout (overwrites channel count, which is inferred from the layout).
	/// @param [in] cl bitset with appropriate @p channel::location flags set.
	void set_channel_layout(const channel::layout& cl)
	{
		channel_layout_ = cl; 
		channel_count_ = static_cast<unsigned>(channel_layout_.count());
	}

	/// @brief Set channel layout based on channel masks (a combination of @p channel::mask flags); overwrites overwrites channel count, which is inferred from the layout.
	/// @param [in] channel_mask channel layout mask.
	void set_channel_config(unsigned channel_mask)
	{
		channel_layout_ = channel::layout(channel_mask); 
		channel_count_ = static_cast<unsigned>(channel_layout_.count());
	}

	/// @return @p true if specified @p channel::location identifier is present in this format's channel layout.
	bool is_channel_present(channel::location::label ch) const {return channel_layout_.test(ch);}

	void set_channel_present(channel::location::label ch, bool present = true)
	{
		channel_layout_.set(ch, present); 
		channel_count_ = static_cast<unsigned>(channel_layout_.count());
	}

	/// @return Current channel layout mask as a combination of @p channel::mask flags.
	unsigned channel_config() const {return static_cast<unsigned>(channel_layout_.to_ulong());}

	/// @return Index of specified channel in current layout or @p channel::not_present if missing.
	unsigned channel_index(channel::location::label ch) const;

	/// @return Number of channels defined by this format's layout.
	unsigned channel_count() const {return channel_count_;}

	/// @brief Set channel count; if channel count is changed, 
	/// channel layout is reset to unknown (no channel location is set).
	/// @param [in] cc number of channels to set.
	void set_channel_count(unsigned cc) 
	{
		if (channel_count_ == cc) 
			return;
		channel_count_ = cc; 
		channel_layout_.reset();
	}

	/// @return @p true it this format has valid (non-zero) number of channels set.
	bool is_channel_count_set() const {return 0 != channel_count_;}

	/// @return location of the channel at specified index.
	channel::location::label channel_at(unsigned index) const;

	unsigned sample_rate() const {return sample_rate_;}
	void set_sample_rate(unsigned sr) { sample_rate_ = sr; }
	bool is_sample_rate_set() const { return 0 != sample_rate_; }

	/// @return Sample format as used by I/O functions or hardware interface.
	const std::string& sample_format() const { return sample_format_; }
	void set_sample_format(const char* sf) { sample_format_ = sf; }
	unsigned sample_bits() const { return sample::bit_size_of(sample_format_.c_str()); }
	sample::type::label sample_type() const {return sample::type_of(sample_format_.c_str());}

	static const format format_audio_cd;

	format(unsigned sample_rate, unsigned channel_count, const char* sample_format = NULL);
	format(unsigned sample_rate, unsigned channel_count, const std::string& sample_format);

	format(unsigned sample_rate, unsigned channel_mask, format_tag_, const char* sample_format = NULL);
	format(unsigned sample_rate, unsigned channel_mask, format_tag_, const std::string& sample_format);

	format();

	template<class TimeMs> 
	unsigned time_ms_to_samples(TimeMs ms) const 
	{return static_cast<unsigned>(sample_rate_ * ms / static_cast<TimeMs>(1000.) + static_cast<TimeMs>(.5));}

	template<class TimeS> 
	unsigned time_to_samples(TimeS s) const 
	{return static_cast<unsigned>(sample_rate_ * s + static_cast<TimeS>(.5));}

#ifdef _WIN32
	void render_waveformatex(void* wfx) const;
	void render_waveformatextensible(void* wfx) const;
#endif // _WIN32

private:
	std::string sample_format_;
	channel::layout channel_layout_;
	unsigned sample_rate_;
	unsigned channel_count_;
};

class DSPXX_API file_format: public format
{
public:

	const std::string& type() const {return type_;}
	void set_type(const char* type) {type_ = type;}
	const char* const extension() const {return file_type::extension_for(type_.c_str());}

	file_format();

	file_format(unsigned sample_rate, unsigned channel_count, const char* sample_format = NULL, const char* type = NULL);
	file_format(unsigned sample_rate, unsigned channel_count, const std::string& sample_format, const std::string& type);

	file_format(unsigned sample_rate, unsigned channel_mask, format_tag_, const char* sample_format = NULL, const char* type = NULL);
	file_format(unsigned sample_rate, unsigned channel_mask, format_tag_, const std::string& sample_format, const std::string& type);

private:
	std::string type_;
};

}}

#endif /* DSP_SND_FORMAT_H_INCLUDED */
