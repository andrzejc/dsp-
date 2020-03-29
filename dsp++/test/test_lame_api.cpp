#include <dsp++/snd/lame/writer.h>
#include <dsp++/snd/lame/error.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>
#include <dsp++/snd/mpeg/property.h>
#include <dsp++/snd/mpg123/reader.h>
#include <dsp++/snd/sndfile/reader.h>

#include "test_utility.h"

#include <gtest/gtest.h>

#include <cstdio>

#if !DSPXX_LAME_DISABLED

namespace dsp { namespace snd { namespace lame {

TEST(lame_api, writer_encoder_properties) {
    writer f;
    file_format fmt{48000, channel::layout::stereo, file_type::label::mpeg, ".abr@256k"};
    test::temp_file tmp;
    EXPECT_NO_THROW(f.open(tmp.name, fmt));
    ASSERT_TRUE(f.supports_properties());
    EXPECT_NO_THROW(f.set_property(mpeg::property::mode, "Standard Stereo"));
    EXPECT_THROW(f.set_property(mpeg::property::layer, "2"), property::error::read_only);
    EXPECT_EQ(*f.property(mpeg::property::vbr), "ABR");
    EXPECT_EQ(*f.property(property::bitrate), "256k");
    EXPECT_NO_THROW(f.set_property(mpeg::property::vbr, "cbr"));
    EXPECT_THROW(f.set_property(mpeg::property::vbr, "foo"), property::error::invalid_value);
    EXPECT_NO_THROW(f.set_property(property::bitrate, "320k"));
    EXPECT_THROW(f.set_property(property::bitrate, "foo"), property::error::invalid_value);
    EXPECT_NO_THROW(f.set_property(mpeg::property::vbr, "abr"));
    EXPECT_EQ(*f.property(property::bitrate), "320k");
}

TEST(lame_api, writer_throws_on_more_than_2_channels) {
    writer f;
    test::temp_file tmp;
    EXPECT_THROW(f.open(tmp.name, {44100, 0, file_type::label::mpeg}), lame::error);
    EXPECT_NO_THROW(f.open(tmp.name, {44100, 1, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {44100, 2, file_type::label::mpeg}));
    EXPECT_THROW(f.open(tmp.name, {44100, channel::layout::s2_1, file_type::label::mpeg}), lame::error);
    EXPECT_THROW(f.open(tmp.name, {44100, channel::layout::s5_1, file_type::label::mpeg}), lame::error);
}

TEST(lame_api, writer_throws_on_unsupported_fs) {
    writer f;
    test::temp_file tmp;
    EXPECT_THROW(f.open(tmp.name, {4410, channel::layout::stereo, file_type::label::mpeg}), lame::error);
    EXPECT_THROW(f.open(tmp.name, {88200, channel::layout::stereo, file_type::label::mpeg}), lame::error);
    EXPECT_THROW(f.open(tmp.name, {96000, channel::layout::stereo, file_type::label::mpeg}), lame::error);
    EXPECT_THROW(f.open(tmp.name, {23997, channel::layout::stereo, file_type::label::mpeg}), lame::error);
    EXPECT_NO_THROW(f.open(tmp.name, {8000, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {11025, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {12000, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {16000, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {22050, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {24000, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {32000, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {44100, 2, file_type::label::mpeg}));
    EXPECT_NO_THROW(f.open(tmp.name, {48000, 2, file_type::label::mpeg}));
}

TEST(lame_api, writer_creates_readable_abr_file) {
    file_format source_format;
    sndfile::reader source;
    source.open(test::data_file("1s_sweep.aif").c_str(), &source_format);

    writer w;
    file_format write_format{
            source_format.sample_rate(),
            source_format.channel_count(),
            file_type::label::mpeg,
            ".abr@256k"
        };
    test::temp_file tmp;
    EXPECT_NO_THROW(w.open(tmp.name, write_format));
    EXPECT_EQ(test::copy_file(source, w), 48000);
    EXPECT_NO_THROW(w.close());

    file_format read_format;
    mpg123::reader r;
    EXPECT_NO_THROW(r.open(tmp.name, &read_format));
    EXPECT_EQ(*r.property(mpeg::property::vbr), "ABR");
    EXPECT_EQ(read_format.sample_rate(), source_format.sample_rate());
    EXPECT_EQ(read_format.channel_count(), source_format.channel_count());
    EXPECT_EQ(r.frame_count(), 48000);
}

TEST(lame_api, writer_creates_cbr_at_proper_bitrate) {
    file_format source_format;
    sndfile::reader source;
    source.open(test::data_file("1s_sweep.aif").c_str(), &source_format);

    writer w;
    file_format write_format{
            source_format.sample_rate(),
            source_format.channel_count(),
            file_type::label::mpeg,
            ".@192k"
        };
    test::temp_file tmp;
    EXPECT_NO_THROW(w.open(tmp.name, write_format));
    EXPECT_EQ(test::copy_file(source, w), 48000);
    EXPECT_NO_THROW(w.close());

    file_format read_format;
    mpg123::reader r;
    EXPECT_NO_THROW(r.open(tmp.name, &read_format));
    EXPECT_EQ(*r.property(mpeg::property::vbr), "CBR");
    EXPECT_EQ(*r.property(property::bitrate), "192k");
    EXPECT_EQ(r.frame_count(), 48000);
}

TEST(lame_api, writer_creates_readable_properties) {
    file_format source_format;
    sndfile::reader source;
    source.open(test::data_file("1s_sweep.aif").c_str(), &source_format);

    writer w;
    file_format write_format{
            source_format.sample_rate(),
            source_format.channel_count(),
            file_type::label::mpeg,
            ".abr@256k"
        };
    test::temp_file tmp;
    EXPECT_NO_THROW(w.open(tmp.name, write_format));
    w.set_property("artist", "dsp++");
    w.set_property("title", "1s sweep in mp3");
    w.set_property("album", "test data");
    w.set_property("album_artist", "dsp++ authors");
    w.set_property("comment", "foo bar");
    w.set_property("bpm", "100");
    w.set_property("key", "Cm");
    w.set_property("track_number", "100");
    w.set_property("track_count", "105");
    w.set_property("disk_count", "3");
    w.set_property("DISCOGS_RELEASE_ID", "foo");
    EXPECT_EQ(test::copy_file(source, w), 48000);
    EXPECT_NO_THROW(w.close());

    file_format read_format;
    mpg123::reader r;
    EXPECT_NO_THROW(r.open(tmp.name, &read_format));
    EXPECT_EQ(*r.property("artist"), "dsp++");
    EXPECT_EQ(*r.property("title"), "1s sweep in mp3");
    EXPECT_EQ(*r.property("album"), "test data");
    EXPECT_EQ(*r.property("album_artist"), "dsp++ authors");
    EXPECT_EQ(*r.property("comment"), "foo bar");
    EXPECT_EQ(*r.property("bpm"), "100");
    EXPECT_EQ(*r.property("key"), "Cm");
    EXPECT_EQ(*r.property("track_number"), "100");
    EXPECT_EQ(*r.property("track_count"), "105");
    EXPECT_FALSE(r.property("disk_number"));
    EXPECT_EQ(*r.property("disk_count"), "3");
    EXPECT_EQ(*r.property("DISCOGS_RELEASE_ID"), "foo");
}

}}}

#endif  // !DSPXX_LAME_DISABLED
