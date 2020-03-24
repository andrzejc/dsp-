#include <dsp++/snd/sndfile/file.h>
#include <dsp++/snd/format.h>

#include <gtest/gtest.h>

namespace dsp { namespace snd { namespace sndifle {

TEST(sndfile_api, file_class_usable) {
    sndfile::file f;
    format fmt;
    f.close();
}

}}}


