#pragma once
#include <dsp++/config.h>

#if !DSPXX_LIBSNDFILE_DISABLED

#include <dsp++/snd/io.h>
#include <dsp++/snd/sndfile/reader.h>
#include <dsp++/snd/sndfile/writer.h>

namespace dsp { namespace snd { namespace sndfile {

struct DSPXX_API file: snd::file, sndfile::reader, sndfile::writer {
    file():
        detail::iobase{mode::rw}
    {}

    using detail::iobase::open;
    using detail::iobase::close;
    using detail::iobase::is_open;
    using detail::iobase::sample_rate;
    using detail::iobase::channel_count;
    using detail::iobase::seekable;
    using detail::iobase::seek;
    using detail::iobase::supports_properties;
    using detail::iobase::property;

    /*!
     * @name Frame-based input.
     */
    ///@{
    /*!
     * @brief Read up to @p count frames from sound file into @p buf.
     * @param buf buffer with with enough space to hold count * channel_count() samples.
     * @param count number of frames to read.
     * @return number of frames read (0 if EOF is reached).
     * @throw sndfile::error or is thrown if error is reported by underlying libsndfile.
     */
    size_t read_frames(float* buf, size_t count) override { return sndfile::reader::read_frames(buf, count); }
    size_t read_frames(short* buf, size_t count) override { return sndfile::reader::read_frames(buf, count); }
    size_t read_frames(int* buf, size_t count) override { return sndfile::reader::read_frames(buf, count); }
    size_t read_frames(double* buf, size_t count) override { return sndfile::reader::read_frames(buf, count); }
    ///@}

    /*!
     * @name Frame-based output.
     */
    ///@{
    /*!
     * @brief Write count frames from buf to the sound file.
     * @param buf buffer holding count * channel_count() samples.
     * @param count number of frames to write.
     * @return number of frames written.
     * @ingroup Frame
     * @throw sndfile::error is thrown if error is reported by underlying libsndfile.
     */
    size_t write_frames(const float* buf, size_t count) override { return sndfile::writer::write_frames(buf, count); }
    size_t write_frames(const short* buf, size_t count) override { return sndfile::writer::write_frames(buf, count); }
    size_t write_frames(const int* buf, size_t count) override { return sndfile::writer::write_frames(buf, count); }
    size_t write_frames(const double* buf, size_t count) override { return sndfile::writer::write_frames(buf, count); }
    ///@}

    void set_property(string_view metadata, string_view value) override { detail::iobase::set_property(metadata, value); }
    void commit() override { detail::iobase::commit(); }
};

}}}

#endif // !DSPXX_LIBSNDFILE_DISABLED
