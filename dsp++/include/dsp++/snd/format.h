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
namespace type { enum label {
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
#define DSP_SND_CHANNEL_MASK(label) label = 1 << (type:: ## label)
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
}; 

//! @param[in] channel_count number of channels.
//! @return "deafult" (most typical) speaker configuration mask for given channel count.
DSPXX_API unsigned default_for(unsigned channel_count);

} // namespace config

//! @brief Bitset for storing channel speaker layouts.
typedef std::bitset<type::count_> layout;

//! @brief Used to denote that given channel type is missing in the layout.
//! @see dsp::snd::format::channel_index()
const unsigned not_present = unsigned(-1);
}

namespace sample {

//! @brief Labels of audio sample formats (these are used primarily for interfacing with dsp::snd::reader and dsp::snd::writer).
namespace label {
const char u8[] = 		"U8";	//!< Unsigned 8-bit integer with offset of 128 linear PCM.
const char s8[] =		"S8"; 	//!< Signed 8-bit integer, linear PCM.
const char s16[] = 		"S16";	//!< Signed 16-bit integer linear PCM.
const char s24[] = 		"S24";	//!< Signed 24-bit integer (packed) linear PCM.
const char s32[] =		"S32";	//!< Signed 32-bit integer linear PCM.
const char f32[] =		"F32";	//!< Floating-point 32-bit (with a non-overdriving range of [-1.0, 1.0]).
const char f64[] =		"F64";	//!< Floating-point 64-bit (with a non-overdriving range of [-1.0, 1.0]).
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
const char wav[] = "wav";
const char aiff[] = "aiff";
const char au[] = "au";
const char raw[] = "raw";
const char wav64[] = "wav64";
const char matlab4[] = "mat4";
const char matlab5[] = "mat5";
const char flac[] = "flac";
const char core_audio[] = "caf";
const char ogg[] = "ogg";
}

DSPXX_API const char* const for_extension(const char* ext);
DSPXX_API const char* const extension_for(const char* cnt);
}

const unsigned sampling_rate_audio_cd = 44100;
const unsigned sampling_rate_phone_narrow = 8000;
const unsigned sampling_rate_phone_wide = 16000;
const unsigned sampling_rate_dat_lp = 32000;
const unsigned sampling_rate_dat = 48000;

class DSPXX_API format
{
public:

	const std::string& sample_format() const {return sample_format_;}

	const channel::layout& channel_layout() const {return channel_layout_;}
	void set_channel_layout(const channel::layout& cl)
	{channel_layout_ = cl; channel_count_ = static_cast<unsigned>(channel_layout_.count());}
	void set_channel_config(unsigned channel_mask)
	{channel_layout_ = channel::layout(static_cast<int>(channel_mask)); channel_count_ = static_cast<unsigned>(channel_layout_.count());}
	bool is_channel_present(channel::type::label ch) const {return channel_layout_.test(ch);}
	void set_channel_present(channel::type::label ch, bool present = true)
	{channel_layout_.set(ch, present); channel_count_ = static_cast<unsigned>(channel_layout_.count());}
	unsigned channel_config() const {return channel_layout_.to_ulong();}
	//! @return index of specified channel in current layout or channel::not_present if missing.
	unsigned channel_index(channel::type::label ch) const;
	unsigned channel_count() const {return channel_count_;}
	void set_channel_count(unsigned cc) {channel_count_ = cc;}
	channel::type::label channel_at(unsigned index) const;

	unsigned sample_rate() const {return sample_rate_;}
	unsigned sample_bits() const {return sample::bit_size_of(sample_format_.c_str());}
	sample::type::label sample_type() const {return sample::type_of(sample_format_.c_str());}

	void set_sample_format(const char* sf) {sample_format_ = sf;}
	void set_sample_rate(unsigned sr) {sample_rate_ = sr;}

	static const format format_audio_cd;

	format(const char* sample_format, unsigned sample_rate, unsigned channel_mask);
	format(const std::string& sample_format, unsigned sample_rate, unsigned channel_mask);

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
	file_format(const char* sample_format, unsigned sample_rate, unsigned channel_mask, const char* type);
	file_format(const std::string& sample_format, unsigned sample_rate, unsigned channel_mask, const std::string& type);

private:
	std::string type_;
};

}}

#endif /* DSP_SND_FORMAT_H_INCLUDED */
