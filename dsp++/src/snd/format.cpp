/*!
 * @file format.cpp
 *
 */
#include <dsp++/snd/format.h>

#include "../utility.h"

#include <boost/format.hpp>

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <algorithm>

namespace dsp { namespace snd {
namespace {
struct file_format_entry {
    const char* label;
    const char* data;
};

const file_format_entry file_extensions[] = {
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

const char* const file_type::extension_for(const char* label) {
    auto e = detail::match_member(file_extensions, &file_format_entry::label, label);
    return (nullptr == e ? nullptr : e->data);
}

const char* const file_type::for_extension(const char* ext) {
    auto e = detail::match_member(file_extensions, &file_format_entry::data, ext);
    return (nullptr == e ? nullptr : e->label);
}

const char* const file_type::mime_subtype_for(const char* label) {
    auto e = detail::match_member(file_mime_types, &file_format_entry::label, label);
    return (nullptr == e ? nullptr : e->data);
}

const char* const file_type::for_mime_subtype(const char* ext) {
    auto e = detail::match_member(file_mime_types, &file_format_entry::data, ext);
    return (nullptr == e ? nullptr : e->label);
}

const format format::audio_cd{sample_rate::audio_cd, channel::layout::stereo, sample::format::S16};

#ifdef _WIN32
}}

#include <windows.h>
#include <mmreg.h>

namespace dsp { namespace snd {
void format::to_WAVEFORMATEX(void* wfx) const {
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
    throw std::runtime_error{boost::str(boost::format("unable to map sample format \"%1%\" to WAVEFORMATEX::wFormatTag") %
        sample_format_
    )};
}

void format::to_WAVEFORMATEXTENSIBLE(void* wfx) const {
    WAVEFORMATEXTENSIBLE* w = static_cast<WAVEFORMATEXTENSIBLE*>(wfx);
    to_WAVEFORMATEX(&w->Format);
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
    w->dwChannelMask = static_cast<DWORD>(channel_layout_.mask());
    w->Samples.wValidBitsPerSample = w->Format.wBitsPerSample;
}

#endif // _WIN32

}}  // namespace dsp::snd
