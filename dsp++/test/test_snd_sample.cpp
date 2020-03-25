#include <dsp++/snd/format.h>

#include <gtest/gtest.h>

namespace dsp { namespace snd {
TEST(snd_sample, type_from_spec) {
    EXPECT_EQ(sample::type_of(sample::format::S16), sample::type::pcm_signed);
    EXPECT_EQ(sample::type_of(sample::format::U8), sample::type::pcm_unsigned);
    EXPECT_EQ(sample::type_of(sample::format::F32), sample::type::ieee_float);
    EXPECT_EQ(sample::type_of("_16.foo"), sample::type::unknown);
    EXPECT_EQ(sample::type_of(".foo"), sample::type::unknown);
    EXPECT_EQ(sample::type_of(".foo@32k"), sample::type::unknown);
    EXPECT_EQ(sample::type_of(""), sample::type::unknown);
    EXPECT_EQ(sample::type_of(nullptr), sample::type::unknown);
}

TEST(snd_sample, bit_size_from_spec) {
    EXPECT_EQ(sample::bit_size_of(sample::format::S16), 16);
    EXPECT_EQ(sample::bit_size_of(sample::format::U8), 8);
    EXPECT_EQ(sample::bit_size_of(sample::format::F32), 32);
    EXPECT_EQ(sample::bit_size_of("_16.foo"), 16);
    EXPECT_EQ(sample::bit_size_of(".foo"), 0);
    EXPECT_EQ(sample::bit_size_of(".foo@32k"), 0);
    EXPECT_EQ(sample::bit_size_of(""), 0);
    EXPECT_EQ(sample::bit_size_of(nullptr), 0);
}
}}