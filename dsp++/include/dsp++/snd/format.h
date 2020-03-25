/*!
 * @file dsp++/snd/format.h
 * @brief Sound (audio) format description.
 */
#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>
#include <dsp++/snd/channel.h>
#include <dsp++/snd/sample.h>

#include <cstddef>
#include <string>

namespace dsp { namespace snd {

/// @brief Audio file type labels, MIME type and file extension mapping.
namespace file_type {

namespace label {
constexpr char wav[] =        "wav";
constexpr char aiff[] =       "aiff";
constexpr char au[] =         "au";
constexpr char raw[] =        "raw";
constexpr char wav64[] =      "wav64";
constexpr char matlab4[] =    "mat4";
constexpr char matlab5[] =    "mat5";
constexpr char flac[] =       "flac";
constexpr char core_audio[] = "caf";
constexpr char ogg[] =        "ogg";
constexpr char htk[] =        "htk";
constexpr char rf64[] =       "rf64";
}

/// @return file type label (a @c file_type::label constant) for specified file extension, or @c nullptr if unknown.
DSPXX_API const char* const for_extension(const char* file_type_spec);
/// @return file extension which should be used with specified file type label (a @c file_type::label constant),
/// or @c nullptr if unknown.
DSPXX_API const char* const extension_for(const char* file_type_spec);
/// @return file type label (a @c file_type::label constant) for specified MIME subtype (assuming MIME type is
/// audio/<i>subtype</i>), or @c nullptr if unknown.
DSPXX_API const char* const for_mime_subtype(const char* file_type_spec);
/// @return MIME subtype which should be used with specified file type label (a @c file_type::label ),
/// or @c nullptr if unknown.
DSPXX_API const char* const mime_subtype_for(const char* file_type_spec);

constexpr char mime_type[] = "audio";

/// @return full MIME type "audio/<subtype>" for given @p file_type_spec.
DSPXX_API string mime_type_for(const char* file_type_spec);

} // namespace file_type


namespace sample_rate {
constexpr unsigned audio_cd = 44100;
constexpr unsigned phone_narrow = 8000;
constexpr unsigned phone_wide = 16000;
constexpr unsigned dat_lp = 32000;
constexpr unsigned dat = 48000;

constexpr unsigned unknown = 0;
}

/// @brief Describes audio stream settings, namely: number (and optionally layout) of channels, sampling rate and
/// sample format.
/// Note that the sample format adheres primarily to the format which is used internally by I/O classes, i.e. the
/// hardware or I/O format, not the sample abstraction used by @p dsp::snd APIs (which tend to use float or double
/// types, mapping to @c sample::format::F32 &amp; @p sample::label::F64).
class DSPXX_API format {
    string sample_format_;
    channel::layout channel_layout_;
    unsigned sample_rate_ = sample_rate::unknown;
    unsigned channel_count_ = 0;

public:
    /// @return Channel layout.
    const channel::layout& channel_layout() const {
        return channel_layout_;
    }

    /// @brief Set channel layout (overwrites channel count, which is inferred from the layout).
    /// @param [in] cl bitset with appropriate @p channel::location flags set.
    void set_channel_layout(const channel::layout& cl) {
        channel_layout_ = cl;
        channel_count_ = static_cast<unsigned>(channel_layout_.count());
    }

    /// @return @p true if specified @p channel::location identifier is present in this format's channel layout.
    bool has_channel(channel::location ch) const {return channel_layout_.has(ch);}

    void set_channel_present(channel::location ch, bool present = true) {
        channel_layout_.set(ch, present);
        channel_count_ = static_cast<unsigned>(channel_layout_.count());
    }

    /// @return Number of channels defined by this format.
    unsigned channel_count() const {
        return channel_count_;
    }

    /// @brief Set channel count; channel layout is reset to unknown (no channel location is set).
    /// @param [in] cc number of channels to set.
    void set_channel_count(unsigned cc) {
        channel_count_ = cc;
        channel_layout_.reset();
    }

    unsigned sample_rate() const {
        return sample_rate_;
    }
    void set_sample_rate(unsigned sr) {
        sample_rate_ = sr;
    }

    /// @return Sample format as used by I/O functions or hardware interface.
    const string& sample_format() const {
        return sample_format_;
    }
    void set_sample_format(string sf) {
        sample_format_ = std::move(sf);
    }
    unsigned sample_bits() const {
        return sample::bit_size_of(sample_format_);
    }
    sample::type sample_type() const {
        return sample::type_of(sample_format_);
    }

    static const format audio_cd;

    format() {}

    format(unsigned sample_rate, unsigned channel_count, string sample_format = {}):
        sample_format_(std::move(sample_format)),
        sample_rate_(sample_rate)
    {
        set_channel_count(channel_count);
    }

    format(unsigned sample_rate, const channel::layout& channel_layout, string sample_format = {}):
        sample_format_(std::move(sample_format)),
        sample_rate_(sample_rate)
    {
        set_channel_layout(channel_layout);
    }

    template<class TimeMs>
    unsigned time_ms_to_samples(TimeMs ms) const
    {return static_cast<unsigned>(sample_rate_ * ms / static_cast<TimeMs>(1000.) + static_cast<TimeMs>(.5));}

    template<class TimeS>
    unsigned time_to_samples(TimeS s) const
    {return static_cast<unsigned>(sample_rate_ * s + static_cast<TimeS>(.5));}

#ifdef _WIN32
    void to_WAVEFORMATEX(void* wfx) const;
    void to_WAVEFORMATEXTENSIBLE(void* wfx) const;
#endif // _WIN32
};

class DSPXX_API file_format: public format {
    string type_;

public:
    const string& type() const {
        return type_;
    }
    void set_type(string type) {
        type_ = std::move(type);
    }
    const char* const extension() const {
        return file_type::extension_for(type_.c_str());
    }
    const char* const mime_subtype() const {
        return file_type::mime_subtype_for(type_.c_str());
    }

    file_format() {}

    file_format(unsigned sample_rate,
                unsigned channel_count,
                string type,
                string sample_format = {}
    ):
        format{sample_rate, channel_count, std::move(sample_format)},
        type_{std::move(type)}
    {}

    file_format(unsigned sample_rate,
                const channel::layout& channel_layout,
                string type,
                string sample_format = {}
    ):
        format{sample_rate, channel_layout, std::move(sample_format)},
        type_{std::move(type)}
    {}
};

}}
