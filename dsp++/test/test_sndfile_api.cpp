#include <dsp++/snd/sndfile/file.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include "test_utility.h"

#include <gtest/gtest.h>

#include <cstdio>

namespace dsp { namespace snd { namespace sndifle {

TEST(sndfile_api, file_usable) {
    sndfile::file f;
    file_format fmt;
    f.open(test::data_file("f32_48k.wav").c_str(), &fmt);
    EXPECT_TRUE(f.is_open());
    EXPECT_EQ(f.sample_rate(), 48000);
    EXPECT_EQ(f.frame_count(), 4800);
    EXPECT_EQ(f.channel_count(), 1);
    EXPECT_TRUE(f.seekable());
    EXPECT_EQ(f.seek(0, SEEK_CUR), 4800);
    EXPECT_EQ(f.seek(100, SEEK_SET), 100);
    EXPECT_TRUE(f.supports_properties());
    EXPECT_EQ(fmt.sample_rate(), 48000);
    EXPECT_EQ(fmt.channel_count(), 1);
    EXPECT_EQ(fmt.channel_layout(), channel::layout::undefined);
    EXPECT_EQ(fmt.sample_format(), sample::format::F32);
    EXPECT_EQ(fmt.sample_type(), sample::type::ieee_float);
    EXPECT_EQ(fmt.sample_bits(), 32);
    EXPECT_EQ(fmt.file_type(), file_type::label::wav);
}

TEST(sndfile_api, reader_usable) {
    sndfile::file f;
    file_format fmt;
    f.open(test::data_file("s24_48k_5ch.wav").c_str(), &fmt);
    EXPECT_TRUE(f.is_open());
    EXPECT_EQ(f.sample_rate(), 48000);
    EXPECT_EQ(f.frame_count(), 4800);
    EXPECT_EQ(f.channel_count(), 5);
    EXPECT_TRUE(f.seekable());
    EXPECT_EQ(f.seek(0, SEEK_CUR), 4800);
    EXPECT_EQ(f.seek(100, SEEK_SET), 100);
    EXPECT_TRUE(f.supports_properties());
    EXPECT_EQ(fmt.sample_rate(), 48000);
    EXPECT_EQ(fmt.channel_count(), 5);
    EXPECT_EQ(fmt.channel_layout(), channel::layout::undefined);
    EXPECT_EQ(fmt.sample_format(), sample::format::S24);
    EXPECT_EQ(fmt.sample_type(), sample::type::pcm_signed);
    EXPECT_EQ(fmt.sample_bits(), 24);
    EXPECT_EQ(fmt.file_type(), file_type::label::wav);
}

TEST(sndfile_api, channel_layout_preserved_on_rewrite) {
    sndfile::reader r;
    file_format rf;
    r.open(test::data_file("s24_48k_5ch.wav").c_str(), &rf);
    ASSERT_EQ(rf.channel_layout(), channel::layout::undefined);
    ASSERT_EQ(r.frame_count(), 4800);
    ASSERT_EQ(r.channel_count(), 5);
    file_format wf = rf;
    wf.set_file_type(file_type::label::aiff);
    wf.set_channel_layout(channel::layout::s5_0);

    test::temp_file tmp;
    sndfile::writer w;
    w.open(tmp.name, &wf);
    ASSERT_TRUE(w.is_open());
    float buf[5 * 480];
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(r.read_frames(buf, 480), 480);
        EXPECT_EQ(w.write_frames(buf, 480), 480);
    }
    w.close();
    r.close();

    r.open(tmp.name, &rf);
    EXPECT_EQ(rf.file_type(), file_type::label::aiff);
    EXPECT_EQ(rf.channel_layout(), channel::layout::s5_0);
}

TEST(sndfile_api, properties_preserved_on_rewrite) {
    sndfile::reader r;
    file_format rf;
    r.open(test::data_file("s16_48k.wav").c_str(), &rf);
    ASSERT_EQ(rf.channel_layout(), channel::layout::undefined);
    ASSERT_EQ(r.frame_count(), 4800);

    file_format wf = rf;
    wf.set_file_type(file_type::label::wav);
    wf.set_channel_layout(channel::layout::stereo);

    test::temp_file tmp;
    sndfile::writer w;
    w.open(tmp.name, &wf);
    w.set_property(property::title, "s16_48k");
    ASSERT_TRUE(w.is_open());
    float buf[5 * 480];
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(r.read_frames(buf, 480), 480);
        EXPECT_EQ(w.write_frames(buf, 480), 480);
    }
    w.close();
    r.close();

    r.open(tmp.name, &rf);
    EXPECT_EQ(rf.file_type(), file_type::label::wav);
    ASSERT_TRUE(r.property(property::title));
    EXPECT_EQ(*r.property(property::title), "s16_48k");
    EXPECT_FALSE(r.property(property::artist));
}

TEST(sndfile_api, reader_format_updated_on_open) {
    sndfile::reader r;
    auto format_pre = r.format();
    r.open(test::data_file("s24_48k_5ch.wav").c_str());
    EXPECT_TRUE(r.is_open());
    EXPECT_NE(format_pre, r.format());
    EXPECT_EQ(r.format().sample_rate(), 48000);
    EXPECT_EQ(r.format().channel_count(), 5);
    EXPECT_EQ(r.format().sample_bits(), 24);
    EXPECT_EQ(r.format().sample_type(), sample::type::pcm_signed);
    EXPECT_EQ(r.format().file_type(), file_type::label::wav);
}

}}}


