/*!
 * @file snd/sndfile_api.cpp
 * @brief Implementation of libsndfile C++ wrapper.
 */
#include <dsp++/config.h>

#if !DSPXX_LIBSNDFILE_DISABLED

#include <dsp++/snd/io.h>
#include <dsp++/snd/sndfile/error.h>
#include <dsp++/snd/sndfile/reader.h>
#include <dsp++/snd/sndfile/writer.h>
#include <dsp++/snd/sndfile/file.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include "../utility.h"

#ifdef _WIN32
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
typedef const wchar_t* LPCWSTR;
#endif // _WIN32

#include <sndfile.h>

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

#include <sys/stat.h>
#include <fcntl.h>

namespace dsp { namespace snd { namespace sndfile {
namespace {

struct file_type_entry {
    const char* label;
    int type;
};

const file_type_entry file_types[] = {
    // use the more featureful WAVEX format for wav when writing new files
    {file_type::label::wav, 		SF_FORMAT_WAVEX},
    {file_type::label::wav, 		SF_FORMAT_WAV},
    {file_type::label::aiff, 		SF_FORMAT_AIFF},
    {file_type::label::au, 			SF_FORMAT_AU},
    {file_type::label::core_audio, 	SF_FORMAT_CAF},
    {file_type::label::flac, 		SF_FORMAT_FLAC},
    {file_type::label::matlab4, 	SF_FORMAT_MAT4},
    {file_type::label::matlab5, 	SF_FORMAT_MAT5},
    {file_type::label::ogg,	 		SF_FORMAT_OGG},
    {file_type::label::raw, 		SF_FORMAT_RAW},
    {file_type::label::w64, 		SF_FORMAT_W64},
    {file_type::label::rf64,        SF_FORMAT_RF64},
};

struct channel_map_entry {
    channel::location our;
    int their;
};

const channel_map_entry channel_map[] = {
    {channel::location::front_left, SF_CHANNEL_MAP_LEFT},
    {channel::location::front_right, SF_CHANNEL_MAP_RIGHT},
    {channel::location::front_center, SF_CHANNEL_MAP_CENTER},
    {channel::location::lfe, SF_CHANNEL_MAP_LFE},
    {channel::location::back_left, SF_CHANNEL_MAP_REAR_LEFT},
    {channel::location::back_right, SF_CHANNEL_MAP_REAR_RIGHT},
    {channel::location::front_left_center, SF_CHANNEL_MAP_FRONT_LEFT_OF_CENTER},
    {channel::location::front_right_center, SF_CHANNEL_MAP_FRONT_RIGHT_OF_CENTER},
    {channel::location::back_center, SF_CHANNEL_MAP_REAR_CENTER},
    {channel::location::side_left, SF_CHANNEL_MAP_SIDE_LEFT},
    {channel::location::side_right, SF_CHANNEL_MAP_SIDE_RIGHT},
    {channel::location::top_center, SF_CHANNEL_MAP_TOP_CENTER},
    {channel::location::top_front_left, SF_CHANNEL_MAP_TOP_FRONT_LEFT},
    {channel::location::top_front_center, SF_CHANNEL_MAP_TOP_FRONT_CENTER},
    {channel::location::top_front_right, SF_CHANNEL_MAP_TOP_FRONT_RIGHT},
    {channel::location::top_back_left, SF_CHANNEL_MAP_TOP_REAR_LEFT},
    {channel::location::top_back_center, SF_CHANNEL_MAP_TOP_REAR_CENTER},
    {channel::location::top_back_right, SF_CHANNEL_MAP_TOP_REAR_RIGHT},
};

int to_sf_format(const file_format& f, bool write_mode) {
    int res = 0;
    auto& ft = f.file_type();
    auto e = dsp::detail::match_member(file_types, &file_type_entry::label, ft);
    if (nullptr != e) {
        res |= e->type;
    } else if (write_mode) {
        throw std::runtime_error{boost::str(boost::format("can't map file type \"%1%\" to libsndfile format") %
            ft
        )};
    }
    switch (f.sample_type()) {
    case sample::type::ieee_float:
        res |= (f.sample_bits() <= 32)
                ? SF_FORMAT_FLOAT
                : SF_FORMAT_DOUBLE;
        break;
    case sample::type::pcm_signed:
        switch (f.sample_bits()) {
        case 8:
            res |= SF_FORMAT_PCM_S8;
            break;
        case 16:
            res |= SF_FORMAT_PCM_16;
            break;
        case 24:
            res |= SF_FORMAT_PCM_24;
            break;
        case 32:
            res |= SF_FORMAT_PCM_32;
            break;
        }
        break;
    case sample::type::pcm_unsigned:
        if (8 == f.sample_bits()) {
            res |= SF_FORMAT_PCM_U8;
        } else {
            throw std::runtime_error{boost::str(boost::format("can't map sample format \"%1%\" to libsndfile format") %
                f.sample_format()
            )};
        }
        break;
    default:
        // TODO check f.sample_format() for sth like ".vorbis.cbr@256k"
        if (res & SF_FORMAT_OGG) {
            res |= SF_FORMAT_VORBIS;
        }
    }
    return res;
}

void from_sf_format(int format, file_format& f) {
    auto e = dsp::detail::match_member(file_types, &file_type_entry::type, format & SF_FORMAT_TYPEMASK);
    if (nullptr != e) {
        f.set_file_type(e->label);
    }

    switch (format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_S8:
        f.set_sample_format(sample::format::S8);
        break;
    case SF_FORMAT_PCM_U8:
        f.set_sample_format(sample::format::U8);
        break;
    case SF_FORMAT_PCM_16:
        f.set_sample_format(sample::format::S16);
        break;
    case SF_FORMAT_PCM_24:
        f.set_sample_format(sample::format::S24);
        break;
    case SF_FORMAT_PCM_32:
        f.set_sample_format(sample::format::S32);
        break;
    case SF_FORMAT_FLOAT:
        f.set_sample_format(sample::format::F32);
        break;
    case SF_FORMAT_DOUBLE:
        f.set_sample_format(sample::format::F64);
        break;
    // TODO Vorbis support?
    }
}
}

namespace detail {
struct iobase::impl {
    std::unique_ptr<SNDFILE, decltype(&sf_close)> sf_{nullptr, sf_close};
    SF_INFO info_;
    snd::file_format format_;
    std::unique_ptr<byte_stream> stream_;
    const iobase::mode mode_;

    impl(iobase::mode m):
        mode_(m)
    {}

    void close() {
        sf_ = nullptr;
        stream_ = nullptr;
        format_ = {};
        info_ = {};
    }

    ~impl() {
        close();
    }

    void throw_error() {
        int code = sf_error(sf_.get());
        assert(SF_ERR_NO_ERROR != code);
        throw error(code, sf_strerror(sf_.get()));
    }

    void read_channel_map(file_format& f) {
        std::unique_ptr<int[]> arr{new int[info_.channels]};
        int res = sf_command(sf_.get(), SFC_GET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * info_.channels);
        if (SF_TRUE != res) {
            f.set_channel_count(info_.channels);
            return;
        }

        channel::layout layout;
        for (int i = 0; i < info_.channels; ++i) {
            if (arr[i] == SF_CHANNEL_MAP_INVALID) {
                continue;
            }
            auto e = dsp::detail::match_member(channel_map, &channel_map_entry::their, arr[i]);
            if (e == nullptr) {
                throw std::runtime_error{boost::str(boost::format("can't match libsndfile channel id %1% to channel::location") %
                    arr[i]
                )};
            }
            layout.set(e->our);
        }
        if (layout.count() == info_.channels) {
            f.set_channel_layout(layout);
        } else {
            f.set_channel_count(info_.channels);
        }
    }

    bool write_channel_map(const file_format& f) {
        std::unique_ptr<int[]> arr{new int[f.channel_count()]};
        unsigned cc = 0;
        for (int i = 0; i < static_cast<int>(channel::location::COUNT); ++i) {
            channel::location loc = static_cast<channel::location>(i);
            if (!f.channel_layout()[loc]) {
                continue;
            }
            auto e = dsp::detail::match_member(channel_map, &channel_map_entry::our, loc);
            if (e == nullptr) {
                throw std::runtime_error{boost::str(boost::format("can't match channel::location %1% to libsndfie channel id") %
                    i
                )};
            }
            arr[cc++] = e->their;
        }
        int res = sf_command(sf_.get(), SFC_SET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * cc);
        return res == SF_TRUE;
    }

    static constexpr int map_mode(iobase::mode m) {
        switch (m) {
        case iobase::mode::write:
            return SFM_WRITE;
        case iobase::mode::rw:
            return SFM_RDWR;
        default:
            return SFM_READ;
        }
    }

    static sf_count_t io_size(void* p) {
        return static_cast<sf_count_t>(static_cast<impl*>(p)->stream_->size());
    }
    static sf_count_t io_seek(sf_count_t offset, int whence, void* p) {
        return static_cast<sf_count_t>(static_cast<impl*>(p)->stream_->seek(static_cast<ssize_t>(offset), whence));
    }
    static sf_count_t io_read(void* buf, sf_count_t count, void* p) {
        return static_cast<sf_count_t>(static_cast<impl*>(p)->stream_->read(buf, static_cast<size_t>(count)));
    }
    static sf_count_t io_write(const void* buf, sf_count_t count, void* p) {
        return static_cast<sf_count_t>(static_cast<impl*>(p)->stream_->write(buf, static_cast<size_t>(count)));
    }
    static sf_count_t io_tell(void* p) {
        return static_cast<sf_count_t>(static_cast<impl*>(p)->stream_->position());
    }

    void open(std::unique_ptr<byte_stream> in, file_format* fmt, void* native_info) {
        if (in == nullptr) {
            throw std::invalid_argument{"in == nullptr"};
        }
        close();

        if (fmt != nullptr) {
            info_ = {};
            info_.samplerate = fmt->sample_rate();
            info_.channels = fmt->channel_count();
            info_.format = to_sf_format(*fmt, mode_ == detail::iobase::mode::write);
        }
        else if (native_info != nullptr) {
            info_ = *static_cast<SF_INFO*>(native_info);
        } else {
            info_ = {};
        }
        stream_ = std::move(in);
        static const SF_VIRTUAL_IO sf_vio = { io_size, io_seek, io_read, io_write, io_tell };
        try {
            sf_.reset(sf_open_virtual(const_cast<SF_VIRTUAL_IO*>(&sf_vio), map_mode(mode_), &info_, this));
            if (nullptr == sf_) {
                throw_error();
            }
            if (fmt != nullptr) {
                format_ = *fmt;
            }
            format_.set_sample_rate(static_cast<unsigned>(info_.samplerate));
            from_sf_format(info_.format, format_);
            if (mode::write != mode_) {
                read_channel_map(format_);
            } else {
                write_channel_map(format_);
            }
        } catch (...) {
            close();
            throw;
        }
        if (fmt != nullptr) {
            *fmt = format_;
        }
        if (native_info != nullptr) {
            *static_cast<SF_INFO*>(native_info) = info_;
        }
    }

    int open_flags() const {
#ifndef O_BINARY
#define O_BINARY 0
#endif
        return mode_ == mode::read
            ? O_RDONLY | O_BINARY
            : mode_ == mode::rw
                ? O_RDWR | O_CREAT | O_BINARY
                : O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
    }

    int open_mode() const {
        return mode_ == mode::read
            ? 0
#ifdef _WIN32
            : _S_IREAD | _S_IWRITE;
#else
            : S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
#endif
    }
};

iobase::iobase(mode m):
    impl_(new impl(m))
{}

iobase::~iobase() {
}

void iobase::open(const char* path, file_format* fmt, void* native_info) {
    impl_->open(std::make_unique<fildes_stream>(path, impl_->open_flags(), impl_->open_mode()), fmt, native_info);
}

#ifdef _WIN32
void iobase::open(const wchar_t* path, file_format* fmt, void* native_info) {
    impl_->open(std::make_unique<fildes_stream>(path, impl_->open_flags(), impl_->open_mode()), fmt, native_info);
}
#endif // _WIN32

void iobase::open(int fd, bool own_fd, file_format* fmt, void* native_info) {
    impl_->open(std::make_unique<fildes_stream>(fd, own_fd), fmt, native_info);
}

void iobase::open(std::unique_ptr<byte_stream> in, file_format* fmt, void* native_info) {
    impl_->open(std::move(in), fmt, native_info);
}

void iobase::open(std::FILE* f, bool own_file, file_format* fmt, void* native_info) {
    impl_->open(std::make_unique<stdio_stream>(f, own_file), fmt, native_info);
}

unsigned iobase::channel_count() const {
    return impl_->info_.channels;
}

unsigned iobase::sample_rate() const {
    return impl_->info_.samplerate;
}

bool iobase::seekable() const {
    return (0 != impl_->info_.seekable);
}

size_t iobase::frame_count() const {
    return static_cast<size_t>(impl_->info_.frames);
}

SNDFILE* iobase::handle() {
    return impl_->sf_.get();
}

size_t iobase::seek(ssize_t frames, int whence) {
    sf_count_t res;
    if ((res = sf_seek(handle(), frames, whence)) < 0) {
        impl_->throw_error();
    }
    return static_cast<size_t>(res);
}

void iobase::close() {
    impl_->close();
}

void iobase::throw_last_error() {
    impl_->throw_error();
}

int iobase::command(int cmd, void* data, int datasize) {
    return sf_command(handle(), cmd, data, datasize);
}

bool iobase::is_open() const {
    return (nullptr != impl_->sf_);
}

const file_format& iobase::format() const {
    return impl_->format_;
}

bool iobase::supports_properties() const {
    return (impl_->info_.format & (SF_FORMAT_AIFF | SF_FORMAT_CAF | SF_FORMAT_FLAC | SF_FORMAT_OGG | SF_FORMAT_WAV | SF_FORMAT_WAVEX | SF_FORMAT_RF64 | SF_FORMAT_XI)) != 0;
}

namespace {
struct property_entry {
    const char* property;
    int id;
};

const property_entry property_map[] = {
    { property::title, SF_STR_TITLE },
    { property::artist, SF_STR_ARTIST },
    { property::album, SF_STR_ALBUM },
    { property::track_number, SF_STR_TRACKNUMBER },
    { property::comment, SF_STR_COMMENT },
    { property::genre, SF_STR_GENRE },
    { property::software, SF_STR_SOFTWARE },
    { property::copyright, SF_STR_COPYRIGHT },
    { property::date, SF_STR_DATE },
};
}

optional<string> iobase::property(string_view prop) {
    auto e = dsp::detail::match_member(property_map, &property_entry::property, prop);
    if (e == nullptr) {
        return {};
    }
    auto res = sf_get_string(handle(), e->id);
    if (nullptr == res) {
        return {};
    }
    return {res};
}

void iobase::set_property(string_view prop, string_view value) {
    auto e = dsp::detail::match_member(property_map, &property_entry::property, prop);
    if (e == nullptr) {
        throw property::error::unsupported{string{prop}, impl_->format_.file_type()};
    }
    int err = sf_set_string(handle(), e->id, string{value}.c_str());
    if (0 != err) {
        // TODO translate to unsupported?
        throw sndfile::error{err, sf_error_number(err)};
    }
}

void iobase::commit() {
    // Function hidden in the reader API
    assert(impl_->mode_ != mode::read);
    sf_write_sync(handle());
    impl_->stream_->flush();
}
}  // namespace detail

#define READ_ITEMS(items, type, name) \
size_t reader::read_ ## items (type* buf, size_t count) \
{ \
    sf_count_t res; \
    if (static_cast<size_t>(res = sf_ ## name ## _ ## type (handle(), buf, static_cast<sf_count_t>(count))) < count \
        && SF_ERR_NO_ERROR != sf_error(handle()) \
    ) { \
        throw_last_error(); \
    } \
    return static_cast<size_t>(res); \
}

#define READ_FRAMES(type) READ_ITEMS(frames, type, readf)
#define READ_SAMPLES(type) READ_ITEMS(samples, type, read)

READ_FRAMES(float);
READ_FRAMES(short);
READ_FRAMES(int);
READ_FRAMES(double);

READ_SAMPLES(float);
READ_SAMPLES(short);
READ_SAMPLES(int);
READ_SAMPLES(double);

#define WRITE_ITEMS(items, type, name) \
size_t writer::write_ ## items (const type* buf, size_t count) \
{ \
    sf_count_t res; \
    if (static_cast<size_t>(res = sf_ ## name ## _ ## type (handle(), buf, static_cast<sf_count_t>(count))) < count \
        && SF_ERR_NO_ERROR != sf_error(handle()) \
    ) { \
        throw_last_error(); \
    } \
    return static_cast<size_t>(res); \
}

#define WRITE_FRAMES(type) WRITE_ITEMS(frames, type, writef)
#define WRITE_SAMPLES(type) WRITE_ITEMS(samples, type, write)

WRITE_FRAMES(float);
WRITE_FRAMES(short);
WRITE_FRAMES(int);
WRITE_FRAMES(double);

WRITE_SAMPLES(float);
WRITE_SAMPLES(short);
WRITE_SAMPLES(int);
WRITE_SAMPLES(double);

}}}

#endif // !DSPXX_LIBSNDFILE_DISABLED
