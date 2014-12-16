/*!
 * @file format.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

//#include "../pch.h"

#include <dsp++/snd/format.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <algorithm>

#include "../utility.h"

#ifdef _WIN32
#include <windows.h>
#endif 

using namespace dsp::snd;

namespace {

struct file_format_entry
{
	const char* label;
	const char* extension;
};

const file_format_entry file_formats[] = {
	{file_type::label::wav, "wav"},
	{file_type::label::aiff, "aiff"},
	{file_type::label::au, "au"},
	{file_type::label::raw, "raw"},
	{file_type::label::wav64, "w64"},
	{file_type::label::matlab4, "mat"},
	{file_type::label::matlab5, "mat"},
	{file_type::label::flac, "flac"},
	{file_type::label::core_audio, "caf"},
	{file_type::label::ogg, "ogg"},
};

}

unsigned channel::config::default_for(unsigned cc) {
	switch (cc) {
	case 1: return mono;
	case 2: return stereo;
	case 3:	return s3_0_stereo;
	case 4: return s4_0_quadro;
	case 5:	return s5_0;
	case 6: return s5_1;
	default:
		return mask::unknown;
	}
}

const char* const file_type::extension_for(const char* label)
{
	const file_format_entry* const e = dsp::detail::match_element(file_formats, &file_format_entry::label, label);
	return (NULL == e ? NULL : e->extension);
}

const char* const file_type::for_extension(const char* ext)
{
	const file_format_entry* const e = dsp::detail::match_element(file_formats, &file_format_entry::extension, ext);
	return (NULL == e ? NULL : e->label);
}

sample::type::label sample::type_of(const char* sf)
{
	size_t len;
	if (NULL == sf || 0 == (len = strlen(sf)))
		return type::unknown;
	switch (tolower(*sf)) {
	case 's':
		return type::pcm_signed;
	case 'u':
		return type::pcm_unsigned;
	case 'f':
		return type::ieee_float;
	default:
		return type::unknown;
	}
}

unsigned sample::bit_size_of(const char* sf)
{
	size_t len;
	if (NULL == sf || 0 == (len = strlen(sf)))
		return sample::size_unknown;
	char type = tolower(*sf++);
	--len;
	if ('s' != type && 'u' != type && 'f' != type)
		return sample::size_unknown;
	char* end;
	int err = 0;
	std::swap(err, errno);
	unsigned long sz = strtoul(sf, &end, 10);
	std::swap(err, errno);
	if (0 != err)
		return sample::size_unknown;
	return static_cast<unsigned>(sz);
}

format::format(unsigned sample_rate, unsigned channel_mask, dsp::snd::format_tag_, const char* sample_format)
 :	sample_format_(NULL != sample_format ? sample_format : "")
 ,	channel_layout_(static_cast<int>(channel_mask))
 ,	sample_rate_(sample_rate)
 ,	channel_count_(static_cast<unsigned>(channel_layout_.count()))
{
}

format::format(unsigned sample_rate, unsigned channel_count, const char* sample_format)
 :	sample_format_(NULL != sample_format ? sample_format : "")
 ,	channel_layout_(channel::mask::unknown)
 ,	sample_rate_(sample_rate)
 ,	channel_count_(channel_count)
{
}

format::format(unsigned sample_rate, unsigned channel_mask, dsp::snd::format_tag_, const std::string& sample_format)
 :	sample_format_(sample_format)
 ,	channel_layout_(static_cast<int>(channel_mask))
 ,	sample_rate_(sample_rate)
 ,	channel_count_(static_cast<unsigned>(channel_layout_.count()))
{
}

format::format(unsigned sample_rate, unsigned channel_count, const std::string& sample_format)
 :	sample_format_(sample_format)
 ,	channel_layout_(channel::mask::unknown)
 ,	sample_rate_(sample_rate)
 ,	channel_count_(channel_count)
{
}

format::format()
 :	channel_layout_(channel::mask::unknown)
 ,	sample_rate_(0)
 ,	channel_count_(0)
{
}

const format format::format_audio_cd(sampling_rate_audio_cd, channel::config::stereo, dsp::snd::format_channel_mask, sample::label::s16);

unsigned format::channel_index(channel::type::label ch) const
{
	if (!is_channel_present(ch))
		return channel::not_present;
	unsigned c = 0;
	for (int i = 0; i < ch; ++i)
		if (channel_layout_.test(i))
			++c;
	return c;
}

file_format::file_format()
{
}

file_format::file_format(unsigned sample_rate, unsigned channel_mask, dsp::snd::format_tag_ tag, const char* sample_format, const char* type)
 :	format(sample_rate, channel_mask, tag, sample_format)
 ,	type_(NULL != type ? type : "")
{
}

file_format::file_format(unsigned sample_rate, unsigned channel_mask, dsp::snd::format_tag_ tag, const std::string& sample_format, const std::string& type)
:	format(sample_rate, channel_mask, tag, sample_format)
,	type_(type)
{
}

file_format::file_format(unsigned sample_rate, unsigned channel_count, const char* sample_format, const char* type)
 :	format(sample_rate, channel_count, sample_format)
 ,	type_(NULL != type ? type : "")
{
}

file_format::file_format(unsigned sample_rate, unsigned channel_count, const std::string& sample_format, const std::string& type)
:	format(sample_rate, channel_count, sample_format)
,	type_(type)
{
}

