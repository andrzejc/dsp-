/*!
 * @file format.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <dsp++/snd/format.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <algorithm>

#include "../utility.h"

using namespace dsp::snd;

namespace {

struct file_format_entry
{
	const char* label;
	const char* extension;
};

const file_format_entry file_formats[] = {
	{ file_type::label::wav, "wav" },
	{ file_type::label::aiff, "aiff" },
	{ file_type::label::aiff, "aif" },
	{ file_type::label::au, "au" },
	{ file_type::label::au, "snd" },
	{ file_type::label::raw, "raw" },
	{ file_type::label::wav64, "w64" },
	{ file_type::label::matlab5, "mat" },
	{ file_type::label::matlab4, "mat" },
	{ file_type::label::flac, "flac" },
	{ file_type::label::core_audio, "caf" },
	{ file_type::label::ogg, "oga" },
	{ file_type::label::ogg, "ogg" },
	{ file_type::label::htk, "htk" },
	{ file_type::label::rf64, "wav"}
};

const file_format_entry file_mime_types[] = {
	{ file_type::label::wav, "vnd.wave" },
	{ file_type::label::wav, "wav" },
	{ file_type::label::wav, "wave" },
	{ file_type::label::wav, "x-wav" },
	{ file_type::label::aiff, "x-aiff" },
	{ file_type::label::aiff, "aiff" },
	{ file_type::label::au, "basic" },
	{ file_type::label::flac, "x-flac" },
	{ file_type::label::core_audio, "x-caf" },
	{ file_type::label::ogg, "ogg" },
	{ file_type::label::rf64, "vnd.wave" }
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
	case 7:	return s7_0;
	case 8: return s7_1;
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

const char* const file_type::mime_subtype_for(const char* label)
{
	const file_format_entry* const e = dsp::detail::match_element(file_mime_types, &file_format_entry::label, label);
	return (NULL == e ? NULL : e->extension);
}

const char* const file_type::for_mime_subtype(const char* ext)
{
	const file_format_entry* const e = dsp::detail::match_element(file_mime_types, &file_format_entry::extension, ext);
	return (NULL == e ? NULL : e->label);
}

sample::type::label sample::type_of(const char* sf)
{
	size_t len;
	if (NULL == sf || 0 == (len = std::strlen(sf)))
		return type::unknown;
	switch (std::tolower(*sf)) {
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
	if (NULL == sf || 0 == (len = std::strlen(sf)))
		return sample::size_unknown;
	int type = std::tolower(*sf++);
	--len;
	if ('s' != type && 'u' != type && 'f' != type)
		return sample::size_unknown;
	char* end;
	int rerr = errno;
	unsigned long sz = std::strtoul(sf, &end, 10);
	int err = errno; errno = rerr;
	if (0 != err)
		return sample::size_unknown;
	return static_cast<unsigned>(sz);
}

const format format::format_audio_cd(sampling_rate_audio_cd,
                                     channel::config::stereo,
                                     dsp::snd::format_channel_mask,
                                     sample::label::s16);

unsigned format::channel_index(channel::location::label ch) const
{
	if (!is_channel_present(ch))
		return channel::not_present;
	unsigned c = 0;
	for (int i = 0; i < ch; ++i)
		if (channel_layout_.test(i))
			++c;
	return c;
}

file_format::file_format(unsigned sample_rate,
                         unsigned channel_mask,
                         dsp::snd::format_tag_ tag,
                         const char* sample_format,
                         const char* type)
 :	format(sample_rate, channel_mask, tag, sample_format)
 ,	type_(NULL != type ? type : "")
{
}

file_format::file_format(unsigned sample_rate,
                         unsigned channel_mask,
                         dsp::snd::format_tag_ tag,
                         const std::string& sample_format,
                         const std::string& type)
 :	format(sample_rate, channel_mask, tag, sample_format)
 ,	type_(type)
{
}

file_format::file_format(unsigned sample_rate,
                         unsigned channel_count,
                         const char* sample_format,
                         const char* type)
 :	format(sample_rate, channel_count, sample_format)
 ,	type_(NULL != type ? type : "")
{
}

file_format::file_format(unsigned sample_rate,
                         unsigned channel_count,
                         const std::string& sample_format,
                         const std::string& type)
:	format(sample_rate, channel_count, sample_format)
,	type_(type)
{
}

#ifdef _WIN32

#include <windows.h>
#include <mmreg.h>

void format::render_waveformatex(void* wfx) const {
	WAVEFORMATEX* w = static_cast<WAVEFORMATEX*>(wfx);
	switch (sample_type()) {
		case sample::type::ieee_float:
			w->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
			break;
		case sample::type::pcm_unsigned:
			if (sample_bits() != 8)
				goto noformat;
			w->wFormatTag = WAVE_FORMAT_PCM;
			break;
		case sample::type::pcm_signed:
			if (sample_bits() == 8)
				goto noformat;
			w->wFormatTag = WAVE_FORMAT_PCM;
			break;
		default:
			goto noformat;
	}
	w->nChannels = channel_count();
	w->nSamplesPerSec = sample_rate();
	w->wBitsPerSample = sample_bits();
	w->nBlockAlign = w->nChannels * w->wBitsPerSample / 8;
	w->nAvgBytesPerSec = w->nBlockAlign * w->nSamplesPerSec;
	w->cbSize = 0;
	return;
	noformat:
	throw std::runtime_error("dsp::snd::format::render_waveformatex(): no direct WAVEFORMATEX::wFormtTag value "
	                         "for dsp::snd::format::sample_type()");
}

void format::render_waveformatextensible(void* wfx) const {
	WAVEFORMATEXTENSIBLE* w = static_cast<WAVEFORMATEXTENSIBLE*>(wfx);
	render_waveformatex(&w->Format);
	switch (w->Format.wFormatTag) {
		case WAVE_FORMAT_PCM:
			w->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			w->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			break;
		case WAVE_FORMAT_IEEE_FLOAT:
			w->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
			w->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
			break;
	}
	w->Format.cbSize = 22;
	w->dwChannelMask = channel_mask();
	w->Samples.wValidBitsPerSample = w->Format.wBitsPerSample;
}

#endif // _WIN32