#include <dsp++/snd/io.h>

#include "test_utility.h"

#include <gtest/gtest.h>

#include <fcntl.h>

#include <cstdio>

namespace dsp { namespace snd {

TEST(snd_io, stdio_stream_opens_existing_file) {
    EXPECT_NO_THROW(stdio_stream(test::data_file("f32_48k.wav").c_str(), "r"));
    EXPECT_NE(stdio_stream(test::data_file("f32_48k.wav").c_str(), "r").file(), nullptr);
    EXPECT_TRUE(stdio_stream(test::data_file("f32_48k.wav").c_str(), "r"));
}

TEST(snd_io, stdio_stream_throws_on_missing_file) {
    EXPECT_THROW(stdio_stream(test::data_file("missing").c_str(), "r"), std::system_error);
}

TEST(snd_io, stdio_stream_read_works) {
    stdio_stream s(test::data_file("f32_48k.wav").c_str(), "rb");
    char buff[5] = {};
    EXPECT_EQ(s.read(buff, 4), 4);
    EXPECT_STREQ(buff, "RIFF");
    EXPECT_EQ(s.position(), 4);
}

TEST(snd_io, stdio_stream_write_works) {
    test::temp_file t;
    {
        stdio_stream w(t.name, "wb");
        EXPECT_EQ(w.write("foo", 3), 3);
        EXPECT_EQ(w.position(), 3);
    }{
        stdio_stream r(t.name, "rb");
        char buff[4] = {};
        EXPECT_EQ(r.read(buff, 3), 3);
        EXPECT_STREQ(buff, "foo");
    }
}

TEST(snd_io, stdio_stream_seek_and_position_works) {
    stdio_stream s(test::data_file("f32_48k.wav").c_str(), "rb");
    EXPECT_EQ(s.position(), 0);
    EXPECT_EQ(s.seek(0, SEEK_CUR), 0);
    char buff[4];
    EXPECT_EQ(s.read(buff, 4), 4);
    EXPECT_EQ(s.seek(0, SEEK_SET), 0);
    EXPECT_EQ(s.position(), 0);
    EXPECT_EQ(s.seek(10, SEEK_CUR), 10);
    EXPECT_EQ(s.position(), 10);
    EXPECT_EQ(s.seek(10, SEEK_CUR), 20);
    EXPECT_EQ(s.position(), 20);
    EXPECT_EQ(s.seek(0, SEEK_END), 19256);
    EXPECT_EQ(s.position(), 19256);
    EXPECT_EQ(s.seek(-256, SEEK_CUR), 19000);
    // Actually POSIX allows this
    // EXPECT_THROW(s.seek(100, SEEK_END), std::system_error);
    // EXPECT_EQ(s.position(), 19000);
    EXPECT_THROW(s.seek(-100, SEEK_SET), std::system_error);
    EXPECT_EQ(s.position(), 19000);
    EXPECT_EQ(s.seek(-19000, SEEK_CUR), 0);
    EXPECT_THROW(s.seek(-1, SEEK_CUR), std::system_error);
}

TEST(snd_io, fildes_stream_opens_existing_file) {
    EXPECT_NO_THROW(fildes_stream(test::data_file("f32_48k.wav").c_str(), O_RDONLY));
    EXPECT_GT(fildes_stream(test::data_file("f32_48k.wav").c_str(), O_RDONLY).fildes(), 0);
    EXPECT_TRUE(fildes_stream(test::data_file("f32_48k.wav").c_str(), O_RDONLY));
}

TEST(snd_io, fildes_stream_throws_on_missing_file) {
    EXPECT_THROW(fildes_stream(test::data_file("missing").c_str(), O_RDONLY), std::system_error);
}

TEST(snd_io, fildes_stream_read_works) {
    fildes_stream s(test::data_file("f32_48k.wav").c_str(), O_RDONLY);
    char buff[5] = {};
    EXPECT_EQ(s.read(buff, 4), 4);
    EXPECT_STREQ(buff, "RIFF");
    EXPECT_EQ(s.position(), 4);
}

TEST(snd_io, fildes_stream_write_works) {
    test::temp_file t;
    {
        fildes_stream w(t.name, O_WRONLY | O_CREAT);
        EXPECT_EQ(w.write("foo", 3), 3);
        EXPECT_EQ(w.position(), 3);
    }{
        fildes_stream r(t.name, O_RDONLY);
        char buff[4] = {};
        EXPECT_EQ(r.read(buff, 3), 3);
        EXPECT_STREQ(buff, "foo");
    }
}

TEST(snd_io, fildes_stream_seek_and_position_works) {
    fildes_stream s(test::data_file("f32_48k.wav").c_str(), O_RDONLY);
    EXPECT_EQ(s.position(), 0);
    EXPECT_EQ(s.seek(0, SEEK_CUR), 0);
    char buff[4];
    EXPECT_EQ(s.read(buff, 4), 4);
    EXPECT_EQ(s.seek(0, SEEK_SET), 0);
    EXPECT_EQ(s.position(), 0);
    EXPECT_EQ(s.seek(10, SEEK_CUR), 10);
    EXPECT_EQ(s.position(), 10);
    EXPECT_EQ(s.seek(10, SEEK_CUR), 20);
    EXPECT_EQ(s.position(), 20);
    EXPECT_EQ(s.seek(0, SEEK_END), 19256);
    EXPECT_EQ(s.position(), 19256);
    EXPECT_EQ(s.seek(-256, SEEK_CUR), 19000);
    // Actually POSIX allows this
    // EXPECT_THROW(s.seek(100, SEEK_END), std::system_error);
    // EXPECT_EQ(s.position(), 19000);
    EXPECT_THROW(s.seek(-100, SEEK_SET), std::system_error);
    EXPECT_EQ(s.position(), 19000);
    EXPECT_EQ(s.seek(-19000, SEEK_CUR), 0);
    EXPECT_THROW(s.seek(-1, SEEK_CUR), std::system_error);
}

}}
