/*!
 * @file resample_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <dsp++/resample.h>
#include <dsp++/snd/sndfile/reader.h>
#include <dsp++/snd/sndfile/writer.h>
#include <dsp++/snd/format.h>
#include <dsp++/float.h>

#include <gtest/gtest.h>

namespace dsp {

TEST(resample, interpolator) {
    const size_t factor = 4;
    snd::file_format ff;
    snd::sndfile::reader r;
    r.open("data/coil_mono.wav", &ff);

    ff.set_sample_rate(ff.sample_rate() * factor);

    snd::sndfile::writer w;
    w.open("data/coil_mono_up4.wav", &ff);

    const size_t len = 1024;
    block_interpolator<float> interp(len, factor, 47, .2);

    while (true) {
        size_t read = r.read_frames(interp.x.begin(), len);
        std::fill(interp.x.begin() + read, interp.x.end(), 0.f);
        interp();
        w.write_frames(interp.y.begin(), read * factor);
        if (read != len)
            break;
    }
}

}
