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

//! Channel type (related to the speaker location). This is ordered according to USB Audio class definition, which is a native representation both in WAVE files and in CoreAudio.
//! @see http://www.usb.org/developers/devclass_docs/audio10.pdf
//! @see http://msdn.microsoft.com/en-us/windows/hardware/gg463006
//! @see https://developer.apple.com/library/mac/#qa/qa1638/_index.html
enum type {
	type_unknown,
	type_left,
	type_right,
	type_center,
	type_lfe,
	type_left_surround,
	type_right_surround,
	type_left_center,
	type_right_center,
	type_surround,
	type_center_surround = type_surround,
	type_side_left,
	type_side_right,
	type_top,
	type_top_center_surround = type_top,
	type_top_front_left,
	type_top_front_center,
	type_top_front_right,
	type_top_back_left,
	type_top_back_center,
	type_top_back_right,

	type_label_count						//!< Number of labels defined so far.
};

#define DSP_SND_CHANNEL_MASK(label) mask_ ## label = 1 << type_ ## label
enum mask {
	DSP_SND_CHANNEL_MASK(unknown),
	DSP_SND_CHANNEL_MASK(left),
	DSP_SND_CHANNEL_MASK(right),
	DSP_SND_CHANNEL_MASK(center),
	DSP_SND_CHANNEL_MASK(lfe),
	DSP_SND_CHANNEL_MASK(left_surround),
	DSP_SND_CHANNEL_MASK(right_surround),
	DSP_SND_CHANNEL_MASK(left_center),
	DSP_SND_CHANNEL_MASK(right_center),
	DSP_SND_CHANNEL_MASK(surround),
	DSP_SND_CHANNEL_MASK(side_left),
	DSP_SND_CHANNEL_MASK(side_right),
	DSP_SND_CHANNEL_MASK(top),
	DSP_SND_CHANNEL_MASK(top_front_left),
	DSP_SND_CHANNEL_MASK(top_front_center),
	DSP_SND_CHANNEL_MASK(top_front_right),
	DSP_SND_CHANNEL_MASK(top_back_left),
	DSP_SND_CHANNEL_MASK(top_back_center),
	DSP_SND_CHANNEL_MASK(top_back_right),
	mask_stereo = mask_left | mask_right,
};

//!Definition of channel labels in multichannel audio formats.
namespace label {
const char mono[] = 			"M";
const char left[] = 			"L";
const char right[] = 			"R";
const char center[] = 			"C";
const char lfe[] = 				"Wo";
const char left_surround[] = 	"Ls";
const char right_surround[] = 	"Rs";
const char left_center[] = 		"Lc";
const char right_center[] = 	"Rc";
const char surround[] = 		"S";	//!< AKA "Rear Center"
const char left_surround1[] = 	"Ls1";
const char right_surround1[] = 	"Rs1";
const char top[] =				"T";

const char layout_stereo[] = 	"LR";
const char layout_surround[] =	"LRC";
const char layout_2_1[] =		"LRWo";
const char layout_3_1[] = 		"LRCWo";
const char layout_quadro[] =	"LRLsRs";
const char layout_5_0[] = 		"LRCLsRs";
const char layout_5_1[] = 		"LRCWoLsRs";
const char layout_6_0[] = 		"LRCLsRsS";
}

DSPXX_API const char* label_for(type ch);
typedef std::bitset<type_label_count> layout;
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

const unsigned size_unknown = 0;
DSPXX_API unsigned bit_size_of(const char* format_label);

enum type {
	type_unknown,
	type_pcm_unsigned,
	type_pcm_signed,
	type_pcm_float,
};

DSPXX_API type type_of(const char* format_label);
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
	{channel_layout_ = cl; channel_count_ = channel_layout_.count();}
	void set_channel_mask(unsigned channel_mask)
	{channel_layout_ = channel::layout(static_cast<int>(channel_mask)); channel_count_ = channel_layout_.count();}
	bool is_channel_present(channel::type ch) const {return channel_layout_.test(ch);}
	void set_channel_present(channel::type ch, bool present = true)
	{channel_layout_.set(ch, present); channel_count_ = channel_layout_.count();}
	unsigned channel_mask() const {return channel_layout_.to_ulong();}
	unsigned channel_index(channel::type ch) const;
	unsigned channel_count() const {return channel_count_;}
	void set_channel_count(unsigned cc) {channel_count_ = cc;}
	channel::type channel_at(unsigned index) const;
	bool format_channel_layout(std::string& layout) const;

	unsigned sample_rate() const {return sample_rate_;}
	unsigned sample_bits() const {return sample::bit_size_of(sample_format_.c_str());}
	sample::type sample_type() const {return sample::type_of(sample_format_.c_str());}

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
