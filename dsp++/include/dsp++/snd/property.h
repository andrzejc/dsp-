#pragma once
#include <dsp++/types.h>
#include <dsp++/export.h>

#include <stdexcept>

namespace dsp { namespace snd { namespace property {

constexpr char title[] = "title";
constexpr char album[] = "album";
constexpr char artist[] = "artist";
constexpr char album_artist[] = "album_artist";
constexpr char track_number[] = "track_number";
constexpr char track_count[] = "track_count";
constexpr char disk_number[] = "disk_number";
constexpr char disk_count[] = "disk_count";
constexpr char date[] = "date";
constexpr char replay_gain[] = "replay_gain";
constexpr char bpm[] = "bpm";
constexpr char genre[] = "genre";
constexpr char comment[] = "comment";
constexpr char software[] = "software";
constexpr char bitrate[] = "bitrate";
constexpr char key[] = "key";

namespace error {
struct DSPXX_API error: std::runtime_error {
    using runtime_error::runtime_error;
};

struct DSPXX_API unsupported: error {
    const string name;
    explicit unsupported(string name, string_view file_format);
};

struct DSPXX_API read_only: error {
    const string name;
    explicit read_only(string name);
};

struct DSPXX_API invalid_value: error {
    const string property;
    const string value;
    explicit invalid_value(string property, string value);
};

}
}}}
