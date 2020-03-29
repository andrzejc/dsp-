#pragma once
#include <dsp++/types.h>
#include <dsp++/snd/io.h>

#include <cstdio>
#include <cstdlib>

namespace dsp { namespace test {

constexpr char PATH_SEPARATOR =
#if defined _WIN32 || defined __CYGWIN__
    '\\';
#else
    '/';
#endif

inline string data_file(const char* file) {
    auto env = std::getenv("DSPXX_TEST_DATA");
    string res = env ? env : "data";
    res += PATH_SEPARATOR;
    res += file;
    return res;
}

struct temp_file {
    char name[L_tmpnam] = {};

    temp_file() {
        std::tmpnam(name);
    }

    ~temp_file() {
        std::remove(name);
    }
};

inline size_t copy_file(snd::reader& r, snd::writer& w) {
    const size_t bufsize = 512;
    size_t total = 0;
    std::unique_ptr<short[]> buf{new short[r.format().channel_count() * bufsize]};
    while (true) {
        size_t read = r.read_frames(&buf[0], bufsize);
        total += w.write_frames(&buf[0], read);
        if (read != bufsize) {
            break;
        }
    }
    return total;
}

}}
