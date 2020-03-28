// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/lame/error.h>

#if !DSPXX_LAME_DISABLED

#include <dsp++/snd/mpeg/property.h>
#include <dsp++/snd/sndfile/iobase.h>
#include <dsp++/snd/lame/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/sample.h>
#include <dsp++/snd/buffer.h>
#include <dsp++/snd/property.h>

#include "../utility.h"

#include <lame/lame.h>

#include <absl/strings/numbers.h>
#include <absl/hash/hash.h>
#include <boost/format.hpp>

#include <cassert>
#include <cstring>
#include <functional>
#include <algorithm>
#include <limits>
#include <memory>
#include <cstdio>
#include <cerrno>
#include <system_error>
#include <unordered_map>

namespace dsp { namespace snd { namespace lame {

namespace {
}

struct lame::writer::impl {
    std::unique_ptr<sndfile::stdio> file;
    using lame_ptr = std::unique_ptr<lame_global_flags, decltype(&lame_close)>;
    lame_ptr handle{nullptr, &lame_close};
    std::vector<uint8_t> buffer;
    file_format format;
    size_t frame_count = 0;
    bool bitstream_init = false;
    const int lame_quality = 2;


    void final_flush() {
        if (!bitstream_init) {
            return;
        }
        buffer.resize(std::max<size_t>(buffer.size(), 7200));
        int err = lame_encode_flush(handle.get(), &buffer[0], buffer.size());
        bitstream_init = false;
        if (err < 0) {
            throw error{err, boost::str(boost::format("lame_encode_flush failed with error %1%") % err)};
        }
        if (err > 0) {
            file->write(&buffer[0], err);
        }
        file->flush();
        lame_mp3_tags_fid(handle.get(), file->file());
    }

    void close() {
        try {
            final_flush();
            file = {};
            format = {};
            frame_count = 0;
        } catch (std::exception&) {
            file = {};
            format = {};
            frame_count = 0;
            throw;
        }
    }

    ~impl() {
        try {
            close();
        } catch (std::exception&) {
        }
    }



    void new_handle(const file_format& fmt) {
        // Initialize 2 encoders the same way, one is a dry run, 2nd one will be initialized on first write
        // after dryrun passes
        handle.reset(lame_init());
        bitstream_init = false;
        int err;
        if (LAME_NOERROR != (err = lame_set_in_samplerate(handle.get(), fmt.sample_rate()))) {
            throw error{err, "lame_set_in_samplerate"};
        }
        if (LAME_NOERROR != (err = lame_set_num_channels(handle.get(), fmt.channel_count()))) {
            throw error{err, "lame_set_num_channels failed"};
        }
        if (LAME_NOERROR != (err = lame_set_quality(handle.get(), lame_quality))) {
            throw error{err, "lame_set_quality failed"};
        }

        auto pos = fmt.sample_format().find('.');
        if (pos != fmt.sample_format().npos) {
            string_view vbr_mode = string_view{fmt.sample_format().data() + pos + 1, fmt.sample_format().length() - pos - 1};
            string_view bitrate;
            if ((pos = vbr_mode.find('@')) != vbr_mode.npos) {
                bitrate = vbr_mode;
                bitrate.remove_prefix(pos + 1);
                vbr_mode.remove_suffix(vbr_mode.length() - pos);
            }
            if (!vbr_mode.empty()) {
                set_property(mpeg::property::vbr, vbr_mode);
            }
            if (!bitrate.empty()) {
                set_property(property::bitrate, bitrate);
            }
        }
    }

    void ensure_bitstream_init() {
        if (bitstream_init) {
            return;
        }
        int err;
        if (LAME_NOERROR != (err = lame_init_params(handle.get()))) {
            throw error{err, "lame_init_params failed"};
        }
        bitstream_init = true;
    }

    static void dryrun_bitstream_init(const file_format& fmt) {
        impl dryrun;
        dryrun.new_handle(fmt);
        dryrun.ensure_bitstream_init();
        // avoid attempt to flush stream in destructor
        dryrun.bitstream_init = false;
    }

    void open(std::unique_ptr<sndfile::stdio> f, const file_format& fmt) {
        close();

        dryrun_bitstream_init(fmt);
        try {
            new_handle(fmt);
            format = fmt;
            file = std::move(f);
        } catch (std::exception&) {
            handle = {};
            throw;
        }
    }

    size_t seek(ssize_t off, int whence) {
        throw std::logic_error{"lame writer is not seekable"};
    }

    optional<string> property(string_view property) {
        static const std::unordered_map<string_view, optional<string>(*)(impl& i), absl::Hash<string_view>> property_map = {
            { mpeg::property::version, [](impl& i) -> optional<string> {
                switch (lame_get_version(i.handle.get())) {
                case 0:
                    return "2.0";
                case 1:
                    return "1.0";
                case 2:
                    return "2.5";
                default:
                    return {};
                }
            }},
            { mpeg::property::mode, [](impl& i) -> optional<string> {
                switch (lame_get_mode(i.handle.get())) {
                case STEREO:
                    return "Standard Stereo";
                case JOINT_STEREO:
                    return "Joint Stereo";
                case DUAL_CHANNEL:
                    return "Dual Channel";
                case MONO:
                    return "Mono";
                default:
                    return {};
                }
            }},
            { mpeg::property::layer, [](impl& i) -> optional<string> {
                return "3";
            }},
            { mpeg::property::vbr, [](impl& i) -> optional<string> {
                switch (lame_get_VBR(i.handle.get())) {
                case vbr_off:
                    return "CBR";
                case vbr_abr:
                    return "ABR";
                case vbr_mt:
                case vbr_rh:
                case vbr_mtrh:
                    return "VBR";
                default:
                    return {};
                }
            }},
            { property::bitrate, [](impl& i) -> optional<string> {
                int res;
                switch (lame_get_VBR(i.handle.get())) {
                case vbr_abr:
                    res = lame_get_VBR_mean_bitrate_kbps(i.handle.get());
                    break;
                default:
                    res = lame_get_brate(i.handle.get());
                }
                if (res > 0) {
                    return boost::str(boost::format("%1%k") % res);
                }
                return {};
            }},
            // { property::title, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::title, &mpg123_id3v1::title);
            // }},
            // { property::artist, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::artist, &mpg123_id3v1::artist);
            // }},
            // { property::album, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::album, &mpg123_id3v1::album);
            // }},
            // { property::date, [](impl& i) {
            //     if (auto res = id3v2_text<'TDAT'>(i)) {
            //         return res;
            //     }
            //     return i.prop_from_fields(&mpg123_id3v2::year, &mpg123_id3v1::year);
            // }},
            // { property::comment, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::comment, &mpg123_id3v1::comment);
            // }},
            // { property::bpm, &id3v2_text<'TBPM'> },
            // { property::track_number, &till_slash<&get_track_number> },
            // { property::track_count, &after_slash<&get_track_number> },
            // { property::disk_number, &till_slash<&id3v2_text<'TPOS'>> },
            // { property::disk_count, &after_slash<&id3v2_text<'TPOS'>> },
            // { property::key, &id3v2_text<'TKEY'> },
            // { property::album_artist, &id3v2_text<'TPE2'> },
            // { property::software, &id3v2_text<'TENC'> },
        };
        auto it = property_map.find(property);
        if (it != property_map.end()) {
            return it->second(*this);
        }
        return {};
    }

    void sync_abr_to_bitrate() {
        int res = lame_get_brate(handle.get());
        if (res > 0) {
            int err = lame_set_VBR_mean_bitrate_kbps(handle.get(), res);
            assert(LAME_NOERROR == err);
            lame_set_VBR_min_bitrate_kbps(handle.get(), res / 2);
            lame_set_VBR_max_bitrate_kbps(handle.get(), res + res / 2);
        }
    }

    void sync_bitrate_to_abr() {
        int res = lame_get_VBR_mean_bitrate_kbps(handle.get());
        if (res > 0) {
            int err =lame_set_brate(handle.get(), res);
            assert(LAME_NOERROR == err);
        }
    }

    template<const char* property>
    static void throw_read_only(impl& i, string_view) {
        throw snd::property::error::read_only{property};
    }

    void set_property(string_view prop, string_view val) {
        static const std::unordered_map<string_view, void(*)(impl& i, string_view val), absl::Hash<string_view>> property_map = {
            { mpeg::property::version, throw_read_only<mpeg::property::version> },
            { mpeg::property::layer, throw_read_only<mpeg::property::layer> },
            { mpeg::property::mode, [](impl& i, string_view val) {
                MPEG_mode mode;
                if (detail::istrequal(val, string_view{"Standard Stereo"})) {
                    mode = STEREO;
                } else if (detail::istrequal(val, string_view{"Joint Stereo"})) {
                    mode = JOINT_STEREO;
                // } else if (detail::istrequal(val, string_view{"Dual Channel"})) {
                //     mode = DUAL_CHANNEL;
                } else if (detail::istrequal(val, string_view{"Mono"})) {
                    mode = MONO;
                } else {
                    throw property::error::invalid_value{mpeg::property::mode, string{val}};
                }
                int err;
                if (LAME_NOERROR != (err = lame_set_mode(i.handle.get(), mode))) {
                    throw error{err, "lame_set_mode failed"};
                }
            }},
            { mpeg::property::vbr, [](impl& i, string_view val) {
                vbr_mode mode;
                if (detail::istrequal(val, string_view{"CBR"})) {
                    mode = vbr_off;
                } else if (detail::istrequal(val, string_view{"ABR"})) {
                    mode = vbr_abr;
                } else if (detail::istrequal(val, string_view{"VBR"})) {
                    mode = vbr_mtrh;
                } else {
                    throw property::error::invalid_value{mpeg::property::vbr, string{val}};
                }
                int old_mode = lame_get_VBR(i.handle.get());
                int err;
                if (LAME_NOERROR != (err = lame_set_VBR(i.handle.get(), mode))) {
                    throw error{err, "lame_set_VBR failed"};
                }
                if (mode != old_mode) {
                    if (mode == vbr_abr) {
                        i.sync_abr_to_bitrate();
                    } else if (old_mode == vbr_abr) {
                        i.sync_bitrate_to_abr();
                    }
                }
            }},
            { property::bitrate, [](impl& i, string_view val) {
                string_view num = val;
                if (!num.empty() && std::tolower(num.back()) == 'k') {
                    num.remove_suffix(1);
                }
                int res = 0;
                if (!absl::SimpleAtoi(num, &res) || res < 0) {
                    throw property::error::invalid_value{property::bitrate, string{val}};
                }
                int err;
                switch (lame_get_VBR(i.handle.get())) {
                case vbr_abr:
                    if (LAME_NOERROR != (err = lame_set_VBR_mean_bitrate_kbps(i.handle.get(), res))) {
                        throw error{err, "lame_set_VBR_mean_bitrate_kbps failed"};
                    }
                    lame_set_VBR_min_bitrate_kbps(i.handle.get(), res / 2);
                    lame_set_VBR_max_bitrate_kbps(i.handle.get(), res + res / 2);
                    break;
                default:
                    if (LAME_NOERROR != (err = lame_set_brate(i.handle.get(), res))) {
                        throw error{err, "lame_set_brate failed"};
                    }
                }
            }},
            // { property::title, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::title, &mpg123_id3v1::title);
            // }},
            // { property::artist, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::artist, &mpg123_id3v1::artist);
            // }},
            // { property::album, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::album, &mpg123_id3v1::album);
            // }},
            // { property::date, [](impl& i) {
            //     if (auto res = id3v2_text<'TDAT'>(i)) {
            //         return res;
            //     }
            //     return i.prop_from_fields(&mpg123_id3v2::year, &mpg123_id3v1::year);
            // }},
            // { property::comment, [](impl& i) {
            //     return i.prop_from_fields(&mpg123_id3v2::comment, &mpg123_id3v1::comment);
            // }},
            // { property::bpm, &id3v2_text<'TBPM'> },
            // { property::track_number, &till_slash<&get_track_number> },
            // { property::track_count, &after_slash<&get_track_number> },
            // { property::disk_number, &till_slash<&id3v2_text<'TPOS'>> },
            // { property::disk_count, &after_slash<&id3v2_text<'TPOS'>> },
            // { property::key, &id3v2_text<'TKEY'> },
            // { property::album_artist, &id3v2_text<'TPE2'> },
            // { property::software, &id3v2_text<'TENC'> },
        };
        auto it = property_map.find(prop);
        if (it != property_map.end()) {
            it->second(*this, val);
        } else {
            throw property::error::unsupported{string{prop}, format.file_type()};
        }
    }
};

writer::writer():
    impl_{new impl}
{}

writer::~writer() {
}

void writer::open(const char* path, file_format& format) {
    impl_->open(std::make_unique<sndfile::stdio>(std::fopen(path, "wb"), true), format);
}

// void writer::open(int fd, bool own_fd, file_format& format) {
//     // impl_->open(std::make_fd, own_fd, format);
// }

void writer::close() {
    impl_->close();
}

bool writer::is_open() const {
    return impl_->handle != nullptr;
}

const file_format& writer::format() const {
    return impl_->format;
}

size_t writer::frame_count() const {
    return impl_->frame_count;
}

unsigned writer::sample_rate() const {
    return impl_->format.sample_rate();
}

unsigned writer::channel_count() const {
    return impl_->format.channel_count();
}

size_t writer::seek(ssize_t offset, int whence) {
    return impl_->seek(offset, whence);
}

optional<string> writer::property(string_view prop) {
    return impl_->property(prop);
}

void writer::set_property(string_view prop, string_view val) {
    impl_->set_property(prop, val);
}

size_t writer::write_frames(const float* buf, size_t count) {
    return 0;
}

size_t writer::write_frames(const short* buf, size_t count) {
    return 0;
}

size_t writer::write_frames(const int* buf, size_t count) {
    return 0;
}

size_t writer::write_frames(const double* buf, size_t count) {
    return 0;
}

void writer::commit() {
}

}}}

#endif // !DSPXX_LAME_DISABLED
