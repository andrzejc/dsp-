#include <dsp++/snd/format.h>
#include <dsp++/snd/buffer.h>

#include <gmock/gmock.h>

#include <climits>

namespace dsp { namespace snd { namespace sample {
TEST(snd_sample, type_from_spec) {
    EXPECT_EQ(type_of(format::S16), type::pcm_signed);
    EXPECT_EQ(type_of(format::U8), type::pcm_unsigned);
    EXPECT_EQ(type_of(format::F32), type::ieee_float);
    EXPECT_EQ(type_of("_16.foo"), type::unknown);
    EXPECT_EQ(type_of(".foo"), type::unknown);
    EXPECT_EQ(type_of(".foo@32k"), type::unknown);
    EXPECT_EQ(type_of(""), type::unknown);
}

TEST(snd_sample, bit_size_from_spec) {
    EXPECT_EQ(bit_size_of(format::S16), 16);
    EXPECT_EQ(bit_size_of(format::U8), 8);
    EXPECT_EQ(bit_size_of(format::F32), 32);
    EXPECT_EQ(bit_size_of("_16.foo"), 16);
    EXPECT_EQ(bit_size_of(".foo"), 0);
    EXPECT_EQ(bit_size_of(".foo@32k"), 0);
    EXPECT_EQ(bit_size_of(""), 0);
}

TEST(snd_sample, cast_float_to_signed) {
    EXPECT_EQ(cast<short>(1.f), SHRT_MAX);
    EXPECT_EQ(cast<short>(-1.f), SHRT_MIN);
    EXPECT_EQ(cast<short>(0.f), 0);
    EXPECT_EQ(cast<short>(.5f), -(SHRT_MIN / 2));
    EXPECT_EQ(cast<short>(-.5f), SHRT_MIN / 2);

    EXPECT_EQ(cast<int16_t>(1.), INT16_MAX);
    EXPECT_EQ(cast<int16_t>(-1.), INT16_MIN);
    EXPECT_EQ(cast<int16_t>(0.), 0);
    EXPECT_EQ(cast<int16_t>(.5), -(INT16_MIN / 2));
    EXPECT_EQ(cast<int16_t>(-.5), INT16_MIN / 2);

    EXPECT_EQ(cast<int32_t>(1.), INT32_MAX);
    EXPECT_EQ(cast<int32_t>(-1.), INT32_MIN);
    EXPECT_EQ(cast<int32_t>(0.), 0);
    EXPECT_EQ(cast<int32_t>(.5), -(INT32_MIN / 2));
    EXPECT_EQ(cast<int32_t>(-.5), INT32_MIN / 2);

    EXPECT_EQ(cast<int8_t>(1.f), INT8_MAX);
    EXPECT_EQ(cast<int8_t>(-1.f), INT8_MIN);
    EXPECT_EQ(cast<int8_t>(0.f), 0);
    EXPECT_EQ(cast<int8_t>(.5f), -(INT8_MIN / 2));
    EXPECT_EQ(cast<int8_t>(-.5), INT8_MIN / 2);
}

TEST(snd_sample, cast_float_to_unsigned) {
    EXPECT_EQ(cast<uint8_t>(1.), 0xFF);
    EXPECT_EQ(cast<uint8_t>(-1.), 0x00);
    EXPECT_EQ(cast<uint8_t>(0.), 0x80);
    EXPECT_EQ(cast<uint8_t>(.5), 0xC0);
    EXPECT_EQ(cast<uint8_t>(-.5), 0x40);

    EXPECT_EQ(cast<uint8_t>(1.f), 0xFF);
    EXPECT_EQ(cast<uint8_t>(-1.f), 0x00);
    EXPECT_EQ(cast<uint8_t>(0.f), 0x80);
    EXPECT_EQ(cast<uint8_t>(.5f), 0xC0);
    EXPECT_EQ(cast<uint8_t>(-.5f), 0x40);

    EXPECT_EQ(cast<uint16_t>(1.), 0xFFFF);
    EXPECT_EQ(cast<uint16_t>(-1.), 0x0000);
    EXPECT_EQ(cast<uint16_t>(0.), 0x8000);
    EXPECT_EQ(cast<uint16_t>(.5), 0xC000);
    EXPECT_EQ(cast<uint16_t>(-.5), 0x4000);

    EXPECT_EQ(cast<uint16_t>(1.f), 0xFFFF);
    EXPECT_EQ(cast<uint16_t>(-1.f), 0x0000);
    EXPECT_EQ(cast<uint16_t>(0.f), 0x8000);
    EXPECT_EQ(cast<uint16_t>(.5f), 0xC000);
    EXPECT_EQ(cast<uint16_t>(-.5f), 0x4000);
}

TEST(snd_sample, cast_unsigned_to_float) {
    EXPECT_EQ(cast<float>((uint8_t)0xff), 0.9921875f);
    EXPECT_EQ(cast<float>((uint8_t)0), -1.f);
    EXPECT_EQ(cast<float>((uint8_t)0x80), 0.f);
    EXPECT_EQ(cast<float>((uint8_t)0xc0), .5f);
    EXPECT_EQ(cast<float>((uint8_t)0x40), -.5f);

    EXPECT_EQ(cast<double>((uint8_t)0xff), 0.9921875);
    EXPECT_EQ(cast<double>((uint8_t)0), -1.);
    EXPECT_EQ(cast<double>((uint8_t)0x80), 0.);
    EXPECT_EQ(cast<double>((uint8_t)0xc0), .5);
    EXPECT_EQ(cast<double>((uint8_t)0x40), -.5);

    EXPECT_EQ(cast<float>((uint16_t)0xffff), 0.999969482421875f);
    EXPECT_EQ(cast<float>((uint16_t)0), -1.f);
    EXPECT_EQ(cast<float>((uint16_t)0x8000), 0.f);
    EXPECT_EQ(cast<float>((uint16_t)0xc000), .5f);
    EXPECT_EQ(cast<float>((uint16_t)0x4000), -.5f);

    EXPECT_EQ(cast<double>((uint16_t)0xffff), 0.999969482421875f);
    EXPECT_EQ(cast<double>((uint16_t)0), -1.);
    EXPECT_EQ(cast<double>((uint16_t)0x8000), 0.);
    EXPECT_EQ(cast<double>((uint16_t)0xc000), .5);
    EXPECT_EQ(cast<double>((uint16_t)0x4000), -.5);
}

TEST(snd_sample, cast_signed_to_float) {
    EXPECT_EQ(cast<float>((int8_t)0x7f), 0.9921875f);
    EXPECT_EQ(cast<float>((int8_t)0x80), -1.f);
    EXPECT_EQ(cast<float>((int8_t)0), 0.f);
    EXPECT_EQ(cast<float>((int8_t)0x40), .5f);
    EXPECT_EQ(cast<float>((int8_t)0xc0), -.5f);

    EXPECT_EQ(cast<double>((int8_t)0x7f), 0.9921875);
    EXPECT_EQ(cast<double>((int8_t)0x80), -1.);
    EXPECT_EQ(cast<double>((int8_t)0), 0.);
    EXPECT_EQ(cast<double>((int8_t)0x40), .5);
    EXPECT_EQ(cast<double>((int8_t)0xc0), -.5);

    EXPECT_EQ(cast<float>((int16_t)0x7fff), 0.999969482421875f);
    EXPECT_EQ(cast<float>((int16_t)0x8000), -1.f);
    EXPECT_EQ(cast<float>((int16_t)0), 0.f);
    EXPECT_EQ(cast<float>((int16_t)0x4000), .5f);
    EXPECT_EQ(cast<float>((int16_t)0xC000), -.5f);

    EXPECT_EQ(cast<double>((int16_t)0x7fff), 0.999969482421875);
    EXPECT_EQ(cast<double>((int16_t)0x8000), -1.);
    EXPECT_EQ(cast<double>((int16_t)0), 0.);
    EXPECT_EQ(cast<double>((int16_t)0x4000), .5);
    EXPECT_EQ(cast<double>((int16_t)0xC000), -.5);
}

TEST(snd_sample, convert_signed_to_float) {
    int16_t in16[] = {0x7fff, 0, (int16_t)0x8000, 0, 0, 0, 0x4000, 0, (int16_t)0xc000, 0};
    float out[10], out_planar[10];
    convert(layout{type::pcm_signed, 2}, sizeof(*in16), in16, layout{type::ieee_float, 4}, sizeof(*out), out, 10);
    EXPECT_THAT(out, ::testing::ElementsAreArray({0.999969482421875f, 0.f, -1.f, 0.f, 0.f, 0.f, .5f, 0.f, -.5f, 0.f}));

    convert(
        layout::S16, buffer::layout::interleaved(2, sizeof(*in16)), in16,
        layout::F32, buffer::layout::planar(5, sizeof(*out_planar)), out_planar,
        5, 2
    );
    EXPECT_THAT(out_planar, ::testing::ElementsAreArray({0.999969482421875f, -1.f, 0.f, .5f, -.5f, 0.f, 0.f, 0.f, 0.f, 0.f}));

    int32_t in32[] = {0x7fffffff, (int32_t)0x80000000, 0, 0x40000000, (int32_t)0xc0000000};
    double dout[5];
    convert(layout{type::pcm_signed, 4}, sizeof(*in32), in32, layout{type::ieee_float, 8}, sizeof(*dout), dout, 5);
    EXPECT_THAT(dout, ::testing::ElementsAreArray({0.9999999995343387, -1., 0., .5, -.5,}));
}

}}}