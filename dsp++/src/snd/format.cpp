/*!
 * @file format.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <dsp++/snd/format.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <algorithm>

#include "../utility.h"

using namespace dsp::snd;

namespace {

const char* const channel_labels[channel::type_label_count] = {
	NULL,
	channel::label::left,
	channel::label::right,
	channel::label::center,
	channel::label::lfe,
	channel::label::left_surround,
	channel::label::right_surround,
	channel::label::left_center,
	channel::label::right_center,
	channel::label::surround,
	channel::label::left_surround1,
	channel::label::right_surround1,
	channel::label::top,
	NULL
};

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

const char* channel::label_for(channel::type ch)
{
	return (ch < channel::type_label_count ? channel_labels[ch] : NULL);
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

sample::type sample::type_of(const char* sf)
{
	size_t len;
	if (NULL == sf || 0 == (len = strlen(sf)))
		return sample::type_unknown;
	switch (tolower(*sf)) {
	case 's':
		return sample::type_pcm_signed;
	case 'u':
		return sample::type_pcm_unsigned;
	case 'f':
		return sample::type_pcm_float;
	default:
		return sample::type_unknown;
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
	unsigned sz = strtoul(sf, &end, 10);
	std::swap(err, errno);
	if (0 != err)
		return sample::size_unknown;
	return sz;
}

format::format(const char* sample_format, unsigned sample_rate, unsigned channel_mask)
 :	sample_format_(NULL != sample_format ? sample_format : "")
 ,	channel_layout_(static_cast<int>(channel_mask))
 ,	sample_rate_(sample_rate)
 ,	channel_count_(static_cast<unsigned>(channel_layout_.count()))
{
}

format::format(const std::string& sample_format, unsigned sample_rate, unsigned channel_mask)
 :	sample_format_(sample_format)
 ,	channel_layout_(static_cast<int>(channel_mask))
 ,	sample_rate_(sample_rate)
 ,	channel_count_(static_cast<unsigned>(channel_layout_.count()))
{
}

format::format()
 :	channel_layout_(channel::mask_unknown)
 ,	sample_rate_(0)
 ,	channel_count_(0)
{
}

const format format::format_audio_cd(sample::label::s16, sampling_rate_audio_cd, channel::mask_stereo);

unsigned format::channel_index(channel::type ch) const
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

file_format::file_format(const char* sample_format, unsigned sample_rate, unsigned channel_mask, const char* type)
 :	format(sample_format, sample_rate, channel_mask)
 ,	type_(NULL != type ? type : "")
{
}

file_format::file_format(const std::string& sample_format, unsigned sample_rate, unsigned channel_mask, const std::string& type)
:	format(sample_format, sample_rate, channel_mask)
,	type_(type)
{
}
