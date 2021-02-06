/*!
 * @file snd/io.cpp
 */
// Force use of 64-bit file APIs on POSIX
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE
#define _LARGEFILE64_SOURCE

#include <dsp++/config.h>
#include <dsp++/snd/io.h>

#include "../utility.h"

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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef DSPXX_OS_FAMILY_UNIX
  #include <stdio.h>
  #include <unistd.h>
#endif

#ifdef _WIN32
  #include <io.h>
  #include <windows.h>
  // __MSVCRT_VERSION__ is defined by MinGW
  #if defined(_MSC_VER) || (__MSVCRT_VERSION__ >= 0x800)
    using doff_t = __int64;
    #define ftello _ftelli64
    #define fseeko _fseeki64
    #define lseek _lseeki64
  #else
    #define doff_t long
    #define ftello ftell
    #define fseeko fseek
  #endif
#elif defined(__ANDROID_API__) && (__ANDROID_API__ < 24)
  using doff_t = long;
  #define ftello ftell
  #define fseeko fseek
#else // !_WIN32
  using doff_t = off_t;
#endif

namespace dsp { namespace snd {
namespace {
std::FILE* throw_on_error(std::FILE* f) {
    if (nullptr == f) {
        assert(0 != errno);
        throw std::system_error{errno, std::generic_category()};
    }
    return f;
}

int throw_on_error(int fd) {
    if (fd < 0) {
        assert(0 != errno);
        throw std::system_error{errno, std::generic_category()};
    }
    return fd;
}
}

byte_stream::position_saver::position_saver(byte_stream& stream):
    stream{stream},
    pos{stream.seek(0, SEEK_CUR)}
{}

byte_stream::position_saver::~position_saver() noexcept(false) {
    try {
        stream.seek(pos, SEEK_SET);
    } catch (...) {
        if (0 == std::uncaught_exceptions()) {
            throw;
        }
    }
}

size_t byte_stream::position() {
    return seek(0, SEEK_CUR);
}

size_t byte_stream::size() {
    position_saver s{*this};
    return seek(0, SEEK_END);
}

stdio_stream::stdio_stream(const char* path, const char* mode):
    stdio_stream{throw_on_error(std::fopen(path, mode)), true}
{}

#ifdef _WIN32
stdio_stream::stdio_stream(const wchar_t* path, const wchar_t* mode):
    stdio_stream{throw_on_error(_wfopen(path, mode))}
{}
#endif

stdio_stream::stdio_stream(int fd, const char* mode):
    stdio_stream{throw_on_error(fdopen(fd, mode)), true}
{}

size_t stdio_stream::size() {
    doff_t res = -1, pos = ::ftello(file_);
    if (pos < 0) {
        goto finally;
    }
    if (0 != ::fseeko(file_, 0, SEEK_END)) {
        goto finally;
    }
    res = ::ftello(file_);
finally:
    if (pos >= 0 && 0 != ::fseeko(file_, pos, SEEK_SET)) {
        res = -1;;
    }
    if (res < 0) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    } else {
        return static_cast<size_t>(res);
    }
}

size_t stdio_stream::seek(ssize_t offset, int whence) {
    if (0 != ::fseeko(file_, static_cast<doff_t>(offset), whence)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    auto res = ::ftello(file_);
    if (res < 0) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return static_cast<size_t>(res);
}

size_t stdio_stream::read(void* buf, size_t size) {
    auto res = std::fread(buf, 1, size, file_);
    if (0 == res && std::ferror(file_)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return res;
}

size_t stdio_stream::write(const void* buf, size_t size) {
    auto res = std::fwrite(buf, 1, size, file_);
    if (0 == res && std::ferror(file_)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return res;
}

size_t stdio_stream::position() {
    auto res = ftello(file_);
    if (-1 == res) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
    return static_cast<size_t>(res);
}

void stdio_stream::flush() {
    if (0 != std::fflush(file_) && std::ferror(file_)) {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
}

stdio_stream::~stdio_stream() {
    if (own_file_) {
        std::fclose(file_);
    }
}

fildes_stream::fildes_stream(const char* path, int flags, int mode):
    fildes_stream{throw_on_error(
            mode != -1
                ? ::open(path, flags, mode)
                : ::open(path, flags)),
        true}
{}

#ifdef _WIN32
fildes_stream::fildes_stream(const wchar_t* path, int flags, int mode):
    fildes_stream{throw_on_error(
            mode != -1
                ? ::_wopen(path, flags, mode)
                : ::_wopen(path, flags)),
        true}
{}
#endif

fildes_stream::~fildes_stream() {
    if (own_fd_ && fd_ >= 0) {
        ::close(fd_);
    }
}

size_t fildes_stream::seek(ssize_t offset, int whence) {
    auto res = ::lseek(fd_, static_cast<doff_t>(offset), whence);
    if (res >= 0) {
        return static_cast<size_t>(res);
    } else {
        assert(errno != 0);
        throw std::system_error{errno, std::generic_category()};
    }
}

size_t fildes_stream::read(void* buf, size_t size) {
    while (true) {
        auto res = ::read(fd_, buf, size);
        if (res < 0) {
            assert(0 != errno);
            if (EINTR != errno) {
                throw std::system_error{errno, std::generic_category()};
            }
        } else {
            return static_cast<size_t>(res);
        }
    }
}

size_t fildes_stream::write(const void* buf, size_t size) {
    while (true) {
        auto res = ::write(fd_, buf, size);
        if (res < 0) {
            assert(0 != errno);
            if (EINTR != errno) {
                throw std::system_error{errno, std::generic_category()};
            }
        } else {
            return static_cast<size_t>(res);
        }
    }
}

void fildes_stream::flush() {
#ifndef _WIN32
    while (true) {
        int res = ::fsync(fd_);
        if (res == 0) {
            return;
        }
        assert(errno != 0);
        if (EINTR != errno) {
            throw std::system_error{errno, std::generic_category()};
        }
    }
#else
    auto handle = _get_osfhandle(fd_);
    if (decltype(handle)(-1) != handle) {
        ::FlushFileBuffers((HANDLE)handle);
    }
#endif
}

}}
