// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/mpg123/error.h>

#if !DSPXX_MPG123_DISABLED

#include <dsp++/snd/mpg123/reader.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/sample.h>
#include <dsp++/snd/buffer.h>
#include <dsp++/snd/property.h>

#include "../utility.h"

#include <mpg123.h>

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

namespace dsp { namespace snd { namespace mpg123 {

namespace {
struct mpg123_encoding_entry {
    mpg123_enc_enum enc;
    const char* sample_format;
};

const mpg123_encoding_entry mpg123_encoding_map[] = {
    { MPG123_ENC_SIGNED_16, sample::format::S16 },
    { MPG123_ENC_UNSIGNED_16, sample::format::U16 },
    { MPG123_ENC_UNSIGNED_8, sample::format::U8 },
    { MPG123_ENC_SIGNED_8, sample::format::S8 },
    { MPG123_ENC_SIGNED_32, sample::format::S32 },
    { MPG123_ENC_UNSIGNED_32, sample::format::U32 },
    { MPG123_ENC_SIGNED_24, sample::format::S24 },
    { MPG123_ENC_UNSIGNED_24, sample::format::U24 },
    { MPG123_ENC_FLOAT_32, sample::format::F32 },
    { MPG123_ENC_FLOAT_64, sample::format::F64 }
};

struct mpg123_initializer {
    int err;
    mpg123_initializer():
        err{mpg123_init()}
    {}

    ~mpg123_initializer() {
        if (MPG123_OK == err) {
            mpg123_exit();
        }
    }
};

mpg123_initializer init_mpg123;
}

struct mpg123::reader::impl {
    std::unique_ptr<mpg123_handle, decltype(&mpg123_delete)> handle{nullptr, mpg123_delete};
    int own_fd = -1;
    file_format format;
    size_t frame_count = 0;
    const size_t buffer_length = 512;
    std::unique_ptr<uint8_t[]> buffer;
    sample::layout sample_layout{sample::type::unknown, 0};
    buffer::layout buffer_layout{0, 0};
    size_t frame_byte_size;
    mpg123_frameinfo frame_info;

    void close() {
        handle = nullptr;
        if (own_fd != -1) {
            ::close(own_fd);
            own_fd = -1;
        }
        buffer = nullptr;
        frame_byte_size = 0;  // don't reuse buffer after it's free
        format = {};
        frame_count = 0;
    }

    ~impl() {
        close();
    }

    void throw_last_error() {
        assert(handle);
        int err = mpg123_errcode(handle.get());
        assert(err != MPG123_OK);
        throw error{err, mpg123_strerror(handle.get())};
    }

    void read_format(bool allow_stream_params_change) {
        long sample_rate = 0;
        int channels = 0, encoding = 0;
        if (MPG123_OK != mpg123_getformat(handle.get(), &sample_rate, &channels, &encoding)) {
            throw_last_error();
        }
        constexpr int supported_encs =  ( MPG123_ENC_SIGNED_16  | MPG123_ENC_UNSIGNED_16
                                        | MPG123_ENC_UNSIGNED_8 | MPG123_ENC_SIGNED_8
                                        | MPG123_ENC_SIGNED_32  | MPG123_ENC_UNSIGNED_32
                                        | MPG123_ENC_SIGNED_24  | MPG123_ENC_UNSIGNED_24
                                        | MPG123_ENC_FLOAT_32   | MPG123_ENC_FLOAT_64    );
        if (encoding != (encoding & supported_encs)) {
            throw std::runtime_error{boost::str(boost::format("mpg123 encoding %1% is not supported") %
                encoding
            )};
        }
        assert(sample_rate != 0);
        assert(channels >= 0);
        assert(channels < 3);
        auto e = dsp::detail::match_member(mpg123_encoding_map, &mpg123_encoding_entry::enc, encoding);
        if (nullptr == e) {
            throw std::runtime_error{boost::str(boost::format("can't match mpg123 encoding %1% to sample format") %
                encoding
            )};
        }
        if (!allow_stream_params_change
            && (sample_rate != format.sample_rate() || channels != format.channel_count())
        ) {
            throw std::runtime_error{"change of sample rate or channel count mid-stream is not supported"};
        }
        format.set_sample_format(e->sample_format);
        format.set_sample_rate(static_cast<unsigned>(sample_rate));
        format.set_channel_layout(channels == 2 ? channel::layout::stereo : channel::layout::mono);
        sample_layout = sample::layout{format.sample_type(), format.sample_bits() / 8};
        buffer_layout = buffer::layout::interleaved(format.channel_count(), sample_layout.container_bytes);
        if (frame_byte_size != buffer_layout.sample_stride) {
            frame_byte_size = buffer_layout.sample_stride;
            buffer.reset(new uint8_t[frame_byte_size * buffer_length]);
        }
    }

    void open_handle(file_format* f) {
        long flags;
        double ignore;
        if (MPG123_OK == mpg123_getparam(handle.get(), MPG123_FLAGS, &flags, &ignore)) {
            // Don't resample, always read at native sample frequency
            flags = flags & ~MPG123_AUTO_RESAMPLE;
            mpg123_param(handle.get(), MPG123_FLAGS, flags, ignore);
        }
        off_t len = mpg123_length(handle.get());
        frame_count = static_cast<size_t>(len >= 0 ? len : 0);
        format.set_file_type(file_type::label::mpeg);

        read_format(true);

        if (MPG123_OK != mpg123_info(handle.get(), &frame_info)) {
            throw_last_error();
        }

        if (f != nullptr) {
            *f = format;
        }
    }

    void open(const char* path, file_format* f = nullptr) {
        close();
        int err = MPG123_OK;
        handle.reset(mpg123_new(nullptr, &err));
        if (!handle) {
            assert(MPG123_OK != err);
            throw error{err, mpg123_plain_strerror(err)};
        }
        try {
            if (MPG123_OK != mpg123_open(handle.get(), path)) {
                throw_last_error();
            }
            open_handle(f);
        } catch (...) {
            close();
            throw;
        }
    }

    size_t seek(ssize_t off, int whence) {
        if (!handle) {
            throw std::logic_error{"mpg123 reader is not open"};
        }
        off_t res;
        if ((res = mpg123_seek(handle.get(), static_cast<off_t>(off), whence)) < 0) {
            throw_last_error();
        }
        return static_cast<size_t>(res);
    }

    size_t read_frames(const sample::layout& out_sl, void* out, size_t out_size) {
        uint8_t* out_data = static_cast<uint8_t*>(out);
        size_t total = 0;
        while (true) {
            if (total == out_size) {
                return total;
            }
            size_t read = 0;
            size_t read_frames = std::min(out_size - total, buffer_length);
            int res = mpg123_read(handle.get(), buffer.get(), read_frames * frame_byte_size, &read);
            if (read > 0) {
                assert((read % frame_byte_size) == 0);
                size_t frames = read / frame_byte_size;
                buffer::layout out_bl = buffer::layout::interleaved(format.channel_count(), out_sl.container_bytes);
                sample::convert(sample_layout, buffer_layout, buffer.get(), out_sl, out_bl, out_data, frames, format.channel_count());
                total += frames;
                out_data += frames * out_bl.sample_stride;
            }
            switch (res) {
            case MPG123_OK:
                continue;
            case MPG123_NEW_FORMAT:
                read_format(false);
                continue;
            case MPG123_DONE:
                return total;
            default:
                throw_last_error();
            }
        }
    }

    template<size_t N>
    optional<string> prop_from_id(mpg123_string *(mpg123_id3v2::* v2), char (mpg123_id3v1::* v1)[N]) {
        mpg123_id3v1* v1p;
        mpg123_id3v2* v2p;
        if (MPG123_OK != mpg123_id3(handle.get(), &v1p, &v2p)) {
            return {};
        }
        if (v2p != nullptr && v2 != nullptr) {
            auto str = (v2p->*v2);
            if (str != nullptr && str->fill != 0) {
                auto res = string{str->p, str->fill};
                if (res.back() == '\0') {
                    res.resize(res.size() - 1);
                }
                return res;
            }
        }
        if (v1p != nullptr && v1 != nullptr) {
            auto str = (v1p->*v1);
            size_t len = 0;
            for (; str[len] != '\0' && len != N; ++len) ;
            if (len != 0) {
                return {{str, len}};
            }
        }
        return {};
    }

    template<uint32_t tag>
    static optional<string> id3v2_text(impl& i) {
        mpg123_id3v1* v1p;
        mpg123_id3v2* v2p;
        if (MPG123_OK != mpg123_id3(i.handle.get(), &v1p, &v2p) || v2p == nullptr) {
            return {};
        }
        for (size_t i = 0; i != v2p->texts; ++i) {
            auto& text = v2p->text[i];
            if ((text.id[0] == static_cast<uint8_t>(tag >> 24))
                && (text.id[1] == static_cast<uint8_t>(tag >> 16))
                && (text.id[2] == static_cast<uint8_t>(tag >> 8))
                && (text.id[3] == static_cast<uint8_t>(tag))
            ) {
                if (text.text.fill != 0) {
                    auto res = string{text.text.p, text.text.fill};
                    if (res.back() == '\0') {
                        res.resize(res.size() - 1);
                    }
                    return res;
                }
            }
        }
        return {};
    }

    optional<string> property(string_view property) {
        static const std::unordered_map<string_view, optional<string>(*)(impl& i), absl::Hash<string_view>> property_map = {
            { "mpeg.version", [](impl& i) -> optional<string> {
                switch (i.frame_info.version) {
                case MPG123_1_0:
                    return "1.0";
                case MPG123_2_0:
                    return "2.0";
                case MPG123_2_5:
                    return "2.5";
                }
                return {};
            }},
            { "mpeg.mode", [](impl& i) -> optional<string> {
                switch (i.frame_info.mode) {
                case MPG123_M_STEREO:
                    return "Standard Stereo";
                case MPG123_M_JOINT:
                    return "Joint Stereo";
                case MPG123_M_DUAL:
                    return "Dual Channel";
                case MPG123_M_MONO:
                    return "Mono";
                }
                return {};
            }},
            { "mpeg.layer", [](impl& i) -> optional<string> {
                return boost::str(boost::format("%1%") % i.frame_info.layer);
            }},
            { "mpeg.vbr", [](impl& i) -> optional<string> {
                switch (i.frame_info.vbr) {
                case MPG123_VBR:
                    return "VBR";
                case MPG123_ABR:
                    return "ABR";
                case MPG123_CBR:
                    return "CBR";
                }
                return {};
            }},
            { property::bitrate, [](impl& i) -> optional<string> {
                return boost::str(boost::format("%1%k") % i.frame_info.bitrate);
            }},
            { property::title, [](impl& i) {
                return i.prop_from_id(&mpg123_id3v2::title, &mpg123_id3v1::title);
            }},
            { property::artist, [](impl& i) {
                return i.prop_from_id(&mpg123_id3v2::artist, &mpg123_id3v1::artist);
            }},
            { property::album, [](impl& i) {
                return i.prop_from_id(&mpg123_id3v2::album, &mpg123_id3v1::album);
            }},
            { property::date, [](impl& i) {
                return i.prop_from_id(&mpg123_id3v2::year, &mpg123_id3v1::year);
            }},
            { property::comment, [](impl& i) {
                return i.prop_from_id(&mpg123_id3v2::comment, &mpg123_id3v1::comment);
            }},
            { property::bpm, &id3v2_text<'TBPM'> },
            { property::track_number, &id3v2_text<'TRCK'> },
        };
        auto it = property_map.find(property);
        if (it != property_map.end()) {
            return it->second(*this);
        }
        return {};
    }
};

reader::reader():
    impl_{new impl}
{}

reader::~reader() {
}

void reader::open(const char* path, file_format* format) {
    impl_->open(path, format);
}

void reader::open(int fd, bool own_fd, file_format* format) {
}

void reader::close() {
    impl_->close();
}

bool reader::is_open() const {
    return impl_->handle != nullptr;
}

const file_format& reader::format() const {
    return impl_->format;
}

size_t reader::frame_count() const {
    return impl_->frame_count;
}

unsigned reader::sample_rate() const {
    return impl_->format.sample_rate();
}

unsigned reader::channel_count() const {
    return impl_->format.channel_count();
}

size_t reader::seek(ssize_t offset, int whence) {
    return impl_->seek(offset, whence);
}

optional<string> reader::property(string_view prop) {
    return impl_->property(prop);
}

size_t reader::read_frames(float* buf, size_t count) {
    return impl_->read_frames(sample::layout::of<float>(), buf, count);
}

size_t reader::read_frames(short* buf, size_t count) {
    return impl_->read_frames(sample::layout::of<short>(), buf, count);
}

size_t reader::read_frames(int* buf, size_t count) {
    return impl_->read_frames(sample::layout::of<int>(), buf, count);
}

size_t reader::read_frames(double* buf, size_t count) {
    return impl_->read_frames(sample::layout::of<short>(), buf, count);
}

}}}

#endif // !DSPXX_MPG123_DISABLED
