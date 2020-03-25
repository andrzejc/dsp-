#include <dsp++/snd/sndfile/file.h>
#include <dsp++/snd/format.h>
#include <dsp++/snd/property.h>

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>

namespace dsp { namespace snd { namespace sndifle {

constexpr char PATH_SEPARATOR =
#if defined _WIN32 || defined __CYGWIN__
    '\\';
#else
    '/';
#endif

inline string test_data(const char* file) {
    auto env = std::getenv("DSPXX_TEST_DATA");
    string res = env ? env : "data";
    res += PATH_SEPARATOR;
    res += file;
    return res;
}

TEST(sndfile_api, file_usable) {
    sndfile::file f;
    file_format fmt;
    f.open(test_data("f32_48k.wav").c_str(), &fmt);
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
    EXPECT_EQ(fmt.type(), file_type::label::wav);

    // ASSERT_TRUE(f.property(property::title));
    // EXPECT_EQ(*f.property(property::title), "f32_48k");
    // ASSERT_TRUE(f.property(property::artist));
    // EXPECT_EQ(*f.property(property::artist), "dsp++");
}

TEST(sndfile_api, reader_usable) {
    sndfile::file f;
    file_format fmt;
    f.open(test_data("s24_48k_5ch.wav").c_str(), &fmt);
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
    EXPECT_EQ(fmt.type(), file_type::label::wav);
}

TEST(sndfile_api, channel_layout_preserved_on_rewrite) {
    sndfile::reader r;
    file_format rf;
    r.open(test_data("s24_48k_5ch.wav").c_str(), &rf);
    ASSERT_EQ(rf.channel_layout(), channel::layout::undefined);
    ASSERT_EQ(r.frame_count(), 4800);
    ASSERT_EQ(r.channel_count(), 5);
    file_format wf = rf;
    wf.set_type(file_type::label::aiff);
    wf.set_channel_layout(channel::layout::s5_0);

    char temp_aiff_file[L_tmpnam];
    std::tmpnam(temp_aiff_file);
    sndfile::writer w;
    w.open(temp_aiff_file, &wf);
    ASSERT_TRUE(w.is_open());
    float buf[5 * 480];
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(r.read_frames(buf, 480), 480);
        EXPECT_EQ(w.write_frames(buf, 480), 480);
    }
    w.close();
    r.close();

    r.open(temp_aiff_file, &rf);
    EXPECT_EQ(rf.type(), file_type::label::aiff);
    EXPECT_EQ(rf.channel_layout(), channel::layout::s5_0);
    r.close();
    std::remove(temp_aiff_file);
}

}}}


