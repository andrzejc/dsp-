#include <dsp++/snd/mpg123/reader.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include "test_utility.h"

#include <gtest/gtest.h>

#include <cstdio>

#if !DSPXX_MPG123_DISABLED

namespace dsp { namespace snd { namespace mpg123 {

TEST(mpg123_api, reader_properties_usable) {
    reader f;
    file_format fmt;
    f.open(test::data_file("s16_48k.mp3").c_str(), &fmt);
    EXPECT_TRUE(f.supports_properties());
    EXPECT_EQ(*f.property("title"), "s16_48k");
    EXPECT_EQ(*f.property("artist"), "dsp++");
    EXPECT_EQ(*f.property("album"), "test samples");
    EXPECT_EQ(*f.property("bpm"), "155");
    EXPECT_EQ(*f.property("track_number"), "42");
    EXPECT_EQ(*f.property("track_count"), "42");
    EXPECT_EQ(*f.property("mpeg.version"), "1.0");
    EXPECT_EQ(*f.property("mpeg.layer"), "3");
    EXPECT_EQ(*f.property("mpeg.vbr"), "CBR");
    EXPECT_EQ(*f.property("bitrate"), "256k");
}

TEST(mpg123_api, reader_usable_2) {
    reader f;
    file_format fmt;
    f.open(test::data_file("s16_44k_2ch.mp3").c_str(), &fmt);
    EXPECT_TRUE(f.is_open());
    EXPECT_EQ(f.sample_rate(), 44100);
    EXPECT_EQ(f.frame_count(), 4410);
    EXPECT_EQ(f.channel_count(), 2);
    EXPECT_TRUE(f.seekable());
    EXPECT_EQ(f.seek(0, SEEK_CUR), 0);
    EXPECT_EQ(f.seek(100, SEEK_SET), 100);
    EXPECT_TRUE(f.supports_properties());
    EXPECT_EQ(fmt.sample_rate(), 44100);
    EXPECT_EQ(fmt.channel_count(), 2);
    EXPECT_EQ(fmt.channel_layout(), channel::layout::stereo);
    EXPECT_EQ(fmt.sample_format(), sample::format::S16);
    EXPECT_EQ(fmt.sample_type(), sample::type::pcm_signed);
    EXPECT_EQ(fmt.sample_bits(), 16);
    EXPECT_EQ(fmt.file_type(), file_type::label::mpeg);
}

template<typename Sample>
struct typed_frame_reading: ::testing::Test {};
TYPED_TEST_SUITE_P(typed_frame_reading);

TYPED_TEST_P(typed_frame_reading, read_0_after_stream_drained) {
    reader f;
    file_format fmt;
    f.open(test::data_file("s16_44k_2ch.mp3").c_str(), &fmt);
    ASSERT_TRUE(f.is_open());
    ASSERT_EQ(f.frame_count(), 4410);
    ASSERT_EQ(f.channel_count(), 2);
    std::unique_ptr<TypeParam> buf{new TypeParam[f.frame_count() * f.channel_count()]};
    EXPECT_EQ(f.read_frames(buf.get(), f.frame_count()), f.frame_count());
    EXPECT_EQ(f.read_frames(buf.get(), f.frame_count()), 0);
}

REGISTER_TYPED_TEST_SUITE_P(typed_frame_reading,
                            read_0_after_stream_drained);

typedef ::testing::Types<float, int, short, double> SampleTypes;
INSTANTIATE_TYPED_TEST_SUITE_P(mpg123_api, typed_frame_reading, SampleTypes);

TEST(mpg123_api, reader_format_updated_on_open) {
    reader r;
    auto format_pre = r.format();
    r.open(test::data_file("s16_48k.mp3").c_str());
    EXPECT_TRUE(r.is_open());
    EXPECT_NE(format_pre, r.format());
    EXPECT_EQ(r.format().sample_rate(), 48000);
    EXPECT_EQ(r.format().channel_count(), 1);
    EXPECT_EQ(r.format().sample_bits(), 16);
    EXPECT_EQ(r.format().sample_type(), sample::type::pcm_signed);
    EXPECT_EQ(r.format().file_type(), file_type::label::mpeg);
}

}}}

#endif  // !DSPXX_MPG123_DISABLED
