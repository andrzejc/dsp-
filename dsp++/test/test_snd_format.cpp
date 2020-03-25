#include <dsp++/snd/format.h>

#include <gmock/gmock.h>

namespace dsp { namespace snd {
TEST(snd_format, to_sample) {
    EXPECT_EQ(stream_format(48000, 2).to_samples(std::chrono::duration<unsigned, std::ratio<1, 10>>{1}), 4800);
    EXPECT_EQ(stream_format(48000, 2).to_samples(std::chrono::duration<unsigned, std::ratio<1, 100>>{10}), 4800);

    EXPECT_EQ(stream_format(48000, 2).to_samples(std::chrono::duration<double, std::micro>{1}), 0.048);

    EXPECT_EQ(stream_format::audio_cd.to_samples(std::chrono::milliseconds{10}), 441);
}

}}