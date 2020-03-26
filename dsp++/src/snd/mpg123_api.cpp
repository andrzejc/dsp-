// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/mpg123/error.h>

#if !DSPXX_MPG123_DISABLED

#include <dsp++/snd/mpg123/reader.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include "../utility.h"

#include <mpg123.h>

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

namespace dsp { namespace snd { namespace mpg123 {

namespace {
struct mpg123_encoding_entry {
    mpg123_enc_enum enc;
    const char* sample_format;
};

const mpg123_encoding_entry mpg123_encoding_map[] = {
    { MPG123_ENC_SIGNED_16, sample::format::S16 },
    { MPG123_ENC_UNSIGNED_16, "U16" },
    { MPG123_ENC_UNSIGNED_8, sample::format::U8 },
    { MPG123_ENC_SIGNED_8, sample::format::S8 },
    { MPG123_ENC_SIGNED_32, sample::format::S32 },
    { MPG123_ENC_UNSIGNED_32, "U32" },
    { MPG123_ENC_SIGNED_24, sample::format::S24 },
    { MPG123_ENC_UNSIGNED_24, "U24" },
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
    std::unique_ptr<mpg123_handle, decltype(&mpg123_delete)> handle_{nullptr, mpg123_delete};
    int own_fd_ = -1;
    file_format format_;
    size_t frame_count_ = 0;

    void close() {
        handle_ = nullptr;
        if (own_fd_ != -1) {
            ::close(own_fd_);
            own_fd_ = -1;
        }
        format_ = {};
        frame_count_ = 0;
    }

    ~impl() {
        close();
    }

    void throw_last_error() {
        assert(handle_);
        int err = mpg123_errcode(handle_.get());
        assert(err != MPG123_OK);
        throw error{err, mpg123_strerror(handle_.get())};
    }

    void post_handle_open(file_format* f) {
        long sample_rate = 0;
        int channels = 0, encoding = 0;
        if (MPG123_OK != mpg123_getformat(handle_.get(), &sample_rate, &channels, &encoding)) {
            throw_last_error();
        }
        assert(sample_rate != 0);
        assert(channels != 0);
        auto e = dsp::detail::match_member(mpg123_encoding_map, &mpg123_encoding_entry::enc, encoding);
        if (nullptr == e) {
            throw std::runtime_error{boost::str(boost::format("can't match mpg123 encoding %1% to sample format") %
                encoding
            )};
        }
        if (MPG123_OK != mpg123_format_none(handle_.get())) {
            throw_last_error();
        }
        if (MPG123_OK != mpg123_format(handle_.get(), sample_rate, channels, encoding)) {
            throw_last_error();
        }
        off_t len = mpg123_length(handle_.get());
        frame_count_ = static_cast<size_t>(len >= 0 ? len : 0);
        format_.set_sample_format(e->sample_format);
        format_.set_sample_rate(static_cast<unsigned>(sample_rate));
        assert(3 != channels);
        format_.set_channel_layout(channels == 2 ? channel::layout::stereo : channel::layout::mono);
        format_.set_file_type(file_type::label::mp3);
        if (f != nullptr) {
            *f = format_;
        }
    }

    void open(const char* path, file_format* f = nullptr) {
        close();
        int err = MPG123_OK;
        handle_.reset(mpg123_new(nullptr, &err));
        if (!handle_) {
            assert(MPG123_OK != err);
            throw error{err, mpg123_plain_strerror(err)};
        }
        if (MPG123_OK != mpg123_open(handle_.get(), path)) {
            throw_last_error();
        }
        post_handle_open(f);
    }

    size_t seek(ssize_t off, int whence) {
        if (!handle_) {
            throw std::logic_error{"mpg123 reader is not open"};
        }
        off_t res;
        if ((res = mpg123_seek(handle_.get(), static_cast<off_t>(off), whence)) < 0) {
            throw_last_error();
        }
        return static_cast<size_t>(res);
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
    return impl_->handle_ != nullptr;
}

const file_format& reader::format() const {
    return impl_->format_;
}

size_t reader::frame_count() const {
    return impl_->frame_count_;
}

unsigned reader::sample_rate() const {
    return impl_->format_.sample_rate();
}

unsigned reader::channel_count() const {
    return impl_->format_.channel_count();
}

size_t reader::seek(ssize_t offset, int whence) {
    return impl_->seek(offset, whence);
}

absl::optional<string> reader::property(string_view prop) {
    return {};
}

size_t reader::read_frames(float* buf, size_t count) {
    return 0;
}

size_t reader::read_frames(short* buf, size_t count) {
    return 0;
}

size_t reader::read_frames(int* buf, size_t count) {
    return 0;
}

size_t reader::read_frames(double* buf, size_t count) {
    return 0;
}

}}}

#endif // !DSPXX_MPG123_DISABLED
