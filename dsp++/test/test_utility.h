#pragma once
#include <dsp++/types.h>

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

}}
