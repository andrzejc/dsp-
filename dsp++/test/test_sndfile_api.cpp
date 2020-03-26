#include <dsp++/snd/sndfile/file.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include "test_utility.h"

#include <gtest/gtest.h>

#include <cstdio>

namespace dsp { namespace snd { namespace sndifle {

size_t copy_file(snd::reader& r, snd::writer& w) {
    const size_t bufsize = 512;
    size_t total = 0;
    std::unique_ptr<float[]> buf{new float[r.format().channel_count() * bufsize]};
    while (true) {
        size_t read = r.read_frames(&buf[0], bufsize);
        total += w.write_frames(&buf[0], read);
        if (read != bufsize) {
            break;
        }
    }
    return total;
}

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

struct channel_layout: ::testing::TestWithParam<const char*> {};

TEST_P(channel_layout, channel_layout_preserved_on_rewrite) {
    sndfile::reader r;
    file_format rf;
    r.open(test::data_file("s24_48k_5ch.wav").c_str(), &rf);
    ASSERT_EQ(rf.channel_layout(), channel::layout::undefined);
    ASSERT_EQ(r.frame_count(), 4800);
    ASSERT_EQ(r.channel_count(), 5);
    file_format wf = rf;
    wf.set_file_type(GetParam());
    wf.set_channel_layout(channel::layout::s5_0);
    if (file_type::label::ogg == wf.file_type()) {
        wf.set_sample_format({});
    }

    test::temp_file tmp;
    sndfile::writer w;
    w.open(tmp.name, &wf);
    ASSERT_TRUE(w.is_open());
    EXPECT_EQ(copy_file(r, w), 4800);
    w.close();
    r.close();

    r.open(tmp.name, &rf);
    EXPECT_EQ(rf.file_type(), GetParam());
    EXPECT_EQ(rf.channel_layout(), channel::layout::s5_0);
}

INSTANTIATE_TEST_SUITE_P(sndfile_api,
                         channel_layout,
                         testing::Values(
                            file_type::label::aiff,
                            file_type::label::core_audio
                        ));

struct properties: ::testing::TestWithParam<const char*> {};

TEST_P(properties, properties_preserved_on_rewrite) {
    sndfile::reader r;
    file_format rf;
    r.open(test::data_file("s16_48k.wav").c_str(), &rf);
    ASSERT_EQ(rf.channel_layout(), channel::layout::undefined);
    ASSERT_EQ(r.frame_count(), 4800);

    file_format wf = rf;
    wf.set_file_type(GetParam());
    if (file_type::label::ogg == wf.file_type()) {
        wf.set_sample_format({});
    }

    test::temp_file tmp;
    sndfile::writer w;
    w.open(tmp.name, &wf);
    w.set_property(property::title, "s16_48k");
    ASSERT_TRUE(w.is_open());
    EXPECT_EQ(copy_file(r, w), 4800);
    w.close();
    r.close();

    r.open(tmp.name, &rf);
    EXPECT_EQ(rf.file_type(), GetParam());
    ASSERT_TRUE(r.property(property::title));
    EXPECT_EQ(*r.property(property::title), "s16_48k");
    EXPECT_FALSE(r.property(property::artist));
}

INSTANTIATE_TEST_SUITE_P(sndfile_api,
                         properties,
                         testing::Values(
                            file_type::label::wav,
                            file_type::label::aiff,
                            file_type::label::core_audio,
// XXX prebuilt libsndfile on Travis crashes here with access violation on Win32
#ifndef _MSC_VER
                            file_type::label::flac,
#endif
                            file_type::label::ogg
                        ));

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


