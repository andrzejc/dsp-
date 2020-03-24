/*!
 * @file snd/io.cpp
 * @brief Implementation of libsndfile C++ wrapper.
 */
// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/sndfile/error.h>

#if !DSPXX_LIBSNDFILE_DISABLED

#include <dsp++/snd/sndfile/reader.h>
#include <dsp++/snd/sndfile/writer.h>
#include <dsp++/snd/format.h>

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

namespace dsp { namespace snd { namespace sndfile {

#ifdef _WIN32
  // __MSVCRT_VERSION__ is defined by MinGW
  #if defined(_MSC_VER) || (__MSVCRT_VERSION__ >= 0x800)
    typedef __int64 off_t;
    #define ftello _ftelli64
    #define fseeko _fseeki64
  #else
    typedef long off_t;
    #define ftello std::ftell
    #define fseeko std::fseek
  #endif
#endif

stdio::stdio(std::FILE* file, bool own_file):
    file_(file),
    own_file_(own_file)
{}

size_t stdio::size() {
    off_t res, pos = ftello(file_);
    if (pos < 0) {
        goto fail;
    }
    if (0 != fseeko(file_, 0, SEEK_END)) {
        goto fail;
    }
    res = ftello(file_);
    if (0 != fseeko(file_, pos, SEEK_SET)) {
        goto fail;
    }
    return static_cast<size_t>(res);
fail:
    assert(errno != 0);
    throw std::system_error{errno, std::generic_category()};
}

size_t stdio::seek(ssize_t offset, int whence) {
    if (0 != fseeko(file_, offset, whence)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return static_cast<size_t>(ftello(file_));
}

size_t stdio::read(void* buf, size_t size) {
    auto res = std::fread(buf, size, 1, file_);
    if (0 == res && std::ferror(file_)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return res;
}

size_t stdio::write(const void* buf, size_t size) {
    auto res = std::fwrite(buf, size, 1, file_);
    if (0 == res && std::ferror(file_)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return res;
}

size_t stdio::position() {
    auto res = ftello(file_);
    if (-1 == res) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return static_cast<size_t>(res);
}

stdio::~stdio() {
    if (own_file_) {
        fclose(file_);
    }
}

namespace {

struct file_type_entry {
    const char* label;
    int type;
};

const file_type_entry file_types[] = {
    {file_type::label::aiff, 		SF_FORMAT_AIFF},
    {file_type::label::au, 			SF_FORMAT_AU},
    {file_type::label::core_audio, 	SF_FORMAT_CAF},
    {file_type::label::flac, 		SF_FORMAT_FLAC},
    {file_type::label::matlab4, 	SF_FORMAT_MAT4},
    {file_type::label::matlab5, 	SF_FORMAT_MAT5},
    {file_type::label::ogg,	 		SF_FORMAT_OGG},
    {file_type::label::raw, 		SF_FORMAT_RAW},
    {file_type::label::wav, 		SF_FORMAT_WAV},
    {file_type::label::wav64, 		SF_FORMAT_W64},
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

int to_sf_format(const file_format& f) {
    int res = 0;
    auto& ft = f.type();
    auto e = dsp::detail::match_member(file_types, &file_type_entry::label, ft);
    if (nullptr != e) {
        res |= e->type;
    } else {
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
        if (res & SF_FORMAT_OGG) {
            res |= SF_FORMAT_VORBIS;
        }
    }
    return res;
}

void from_sf_format(int format, file_format& f) {
    auto e = dsp::detail::match_member(file_types, &file_type_entry::type, format & SF_FORMAT_TYPEMASK);
    if (nullptr != e) {
        f.set_type(e->label);
    } else if (format & SF_FORMAT_WAVEX) {
        f.set_type(file_type::label::wav);
    }

    switch (format & SF_FORMAT_SUBMASK) {
    case SF_FORMAT_PCM_S8:
        f.set_sample_format(sample::format::s8);
        break;
    case SF_FORMAT_PCM_U8:
        f.set_sample_format(sample::format::u8);
        break;
    case SF_FORMAT_PCM_16:
        f.set_sample_format(sample::format::s16);
        break;
    case SF_FORMAT_PCM_24:
        f.set_sample_format(sample::format::s24);
        break;
    case SF_FORMAT_PCM_32:
        f.set_sample_format(sample::format::s32);
        break;
    case SF_FORMAT_FLOAT:
        f.set_sample_format(sample::format::f32);
        break;
    case SF_FORMAT_DOUBLE:
        f.set_sample_format(sample::format::f64);
        break;
    }
}
}

namespace detail {
struct iobase::impl {
    SNDFILE* sf_ = nullptr;
    SF_INFO info_;
    io* io_ = nullptr;
    bool own_io_ = false;
    const iobase::mode mode_;

    impl(iobase::mode m):
        mode_(m)
    {}

    void close() {
        if (nullptr != sf_) {
            sf_close(sf_);
            sf_ = nullptr;
        }
        if (own_io_) {
            delete io_;
        }
        own_io_ = false;
        io_ = nullptr;
    }

    ~impl() {
        close();
    }

    void throw_error() {
        int code = sf_error(sf_);
        assert(SF_ERR_NO_ERROR != code);
        throw error(code, sf_strerror(sf_));
    }

    void init_info(file_format* f, SF_INFO* info) {
        if (f != nullptr) {
            std::memset(&info_, 0, sizeof(info_));
            info_.samplerate = f->sample_rate();
            info_.channels = f->channel_count();
            info_.format = to_sf_format(*f);
        }
        else if (info != nullptr) {
            info_ = *info;
        } else {
            std::memset(&info_, 0, sizeof(info_));
        }
    }

    void read_channel_map(file_format& f) {
        std::unique_ptr<int[]> arr{new int[info_.channels]};
        int res = sf_command(sf_, SFC_GET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * info_.channels);
        if (0 != res) {
            return;
        }

        channel::layout layout;
        for (int i = 0; i < info_.channels; ++i) {
            auto e = dsp::detail::match_member(channel_map, &channel_map_entry::their, arr[i]);
            if (e == nullptr) {
                throw std::runtime_error{boost::str(boost::format("can't match libsndfile channel id %1% to channel::location") %
                    arr[i]
                )};
            }
            layout.set(e->our);
        }
        f.set_channel_layout(layout);
    }

    void write_channel_map(const file_format& f) {
        std::unique_ptr<int[]> arr{new int[f.channel_count()]};
        unsigned cc = 0;
        for (int i = 0; i < static_cast<int>(channel::location::COUNT); ++i) {
            channel::location loc = static_cast<channel::location>(i);
            if (!f.has_channel(loc)) {
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
        sf_command(sf_, SFC_SET_CHANNEL_MAP_INFO, &arr[0], sizeof(int) * cc);
    }

    void fill_info(file_format* f, SF_INFO* info) {
        if (f != nullptr) {
            f->set_sample_rate(static_cast<unsigned>(info_.samplerate));
            f->set_channel_count(static_cast<unsigned>(info_.channels));
            from_sf_format(info_.format, *f);
            if (mode::write != mode_) {
                read_channel_map(*f);
            } else {
                write_channel_map(*f);
            }
        }
        if (info != nullptr) {
            *info = info_;
        }
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

    void open(const char* path, file_format* fmt, void* native_info) {
        close();
        init_info(fmt, static_cast<SF_INFO*>(native_info));
        if (nullptr == (sf_ =  sf_open(path, map_mode(mode_), &info_))) {
            throw_error();
        }
        fill_info(fmt, static_cast<SF_INFO*>(native_info));
    }

    void open(int fd, bool own_fd, file_format* fmt, void* native_info) {
        close();
        init_info(fmt, static_cast<SF_INFO*>(native_info));
        if (nullptr == (sf_ = sf_open_fd(fd, map_mode(mode_), &info_, own_fd ? 1 : 0))) {
            throw_error();
        }
        fill_info(fmt, static_cast<SF_INFO*>(native_info));
    }

#ifdef _WIN32
    void open(const wchar_t* path, file_format* fmt, void* native_info) {
        close();
        init_info(fmt, static_cast<SF_INFO*>(native_info));
        if (nullptr == (sf_ =  sf_wchar_open(path, map_mode(mode_), &info_))) {
            throw_error();
        }
        fill_info(fmt, static_cast<SF_INFO*>(native_info));
    }
#endif // _WIN32

    static sf_count_t io_size(void* p) {
        return static_cast<sf_count_t>(static_cast<io*>(p)->size());
    }
    static sf_count_t io_seek(sf_count_t offset, int whence, void* p) {
        return static_cast<sf_count_t>(static_cast<io*>(p)->seek(static_cast<ssize_t>(offset), whence));
    }
    static sf_count_t io_read(void* buf, sf_count_t count, void* p) {
        return static_cast<sf_count_t>(static_cast<io*>(p)->read(buf, static_cast<size_t>(count)));
    }
    static sf_count_t io_write(const void* buf, sf_count_t count, void* p) {
        return static_cast<sf_count_t>(static_cast<io*>(p)->write(buf, static_cast<size_t>(count)));
    }
    static sf_count_t io_tell(void* p) {
        return static_cast<sf_count_t>(static_cast<io*>(p)->position());
    }
    static const SF_VIRTUAL_IO sf_vio;

    void open(io* in, bool own_io, file_format* fmt, void* native_info) {
        if (in == nullptr) {
            throw std::invalid_argument("in == nullptr");
        }
        close();
        init_info(fmt, static_cast<SF_INFO*>(native_info));
        if (nullptr == (sf_ = sf_open_virtual(const_cast<SF_VIRTUAL_IO*>(&sf_vio),
                map_mode(mode_), &info_, in)))
        {
            if (own_io) {
                delete in;
            }
            throw_error();
        }
        io_ = in;
        own_io_ = own_io;
        fill_info(fmt, static_cast<SF_INFO*>(native_info));
    }
};

const SF_VIRTUAL_IO iobase::impl::sf_vio = {
        &impl::io_size,
        &impl::io_seek,
        &impl::io_read,
        &impl::io_write,
        &impl::io_tell
};

iobase::iobase(mode m):
    impl_(new impl(m))
{}

iobase::~iobase() {
}

void iobase::open(const char* path, file_format* fmt, void* native_info) {
    impl_->open(path, fmt, native_info);
}

#ifdef _WIN32
void iobase::open(const wchar_t* path, file_format* fmt, void* native_info) {
    impl_->open(path, fmt, native_info);
}
#endif // _WIN32

void iobase::open(int fd, bool own_fd, file_format* fmt, void* native_info) {
    impl_->open(fd, own_fd, fmt, native_info);
}

void iobase::open(io* in, bool own_io, file_format* fmt, void* native_info) {
    impl_->open(in, own_io, fmt, native_info);
}

void iobase::open(std::FILE* f, bool own_file, file_format* fmt, void* native_info) {
    open(new stdio(f, own_file), true, fmt, native_info);
}

unsigned iobase::channel_count() const {
    return impl_->info_.channels;
}

unsigned iobase::sample_rate() const {
    return impl_->info_.samplerate;
}

bool iobase::is_seekable() const {
    return (0 != impl_->info_.seekable);
}

size_t iobase::frame_count() const {
    return static_cast<size_t>(impl_->info_.frames);
}

SNDFILE* iobase::handle() {
    return impl_->sf_;
}

size_t iobase::seek(ssize_t frames, int whence) {
    sf_count_t res;
    if ((res = sf_seek(impl_->sf_, frames, whence)) < 0) {
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
    return sf_command(impl_->sf_, cmd, data, datasize);
}

bool iobase::is_open() const {
    return (nullptr != impl_->sf_);
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
