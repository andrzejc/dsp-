/*!
 * @file dsp++/snd/sndfile/reader.h
 * @brief Declaration of class dsp::snd::sndfile::reader.
 */
#pragma once
#include <dsp++/config.h>

#ifndef DSPXX_LIBSNDFILE_DISABLED
#include <dsp++/types.h>
#include <dsp++/snd/io.h>
#include <dsp++/snd/sndfile/iobase.h>

namespace dsp { namespace snd { namespace sndfile {

//! @brief Simple interface for reading sound files (uses libsndfile as a back-end).
// TODO make detail::iobase private
class DSPXX_API reader: public virtual snd::reader, protected virtual detail::iobase {
public:
    /*!
     * @brief Constructor.
     * To open the actual stream use one of the open() methods.
     */
    reader():
        detail::iobase{mode::read}
    {}

    using detail::iobase::open;
    using detail::iobase::close;
    using detail::iobase::is_open;
    using detail::iobase::format;
    using detail::iobase::sample_rate;
    using detail::iobase::frame_count;
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
    size_t read_frames(float* buf, size_t count) override;
    size_t read_frames(short* buf, size_t count) override;
    size_t read_frames(int* buf, size_t count) override;
    size_t read_frames(double* buf, size_t count) override;
    ///@}

    /*!
     * @name Sample-based input.
     */
    ///@{
    /*!
     * @brief Read up to @p count samples from sound file into @p buf.
     * @param buf buffer with with enough space to hold count samples.
     * @param count number of samples to read (must be integer multiply of channel_count()).
     * @return number of samples read (0 if EOF is reached).
     * @throw sndfile::error is thrown if error is reported by underlying libsndfile.
     */
    size_t read_samples(float* buf, size_t count);
    size_t read_samples(short* buf, size_t count);
    size_t read_samples(int* buf, size_t count);
    size_t read_samples(double* buf, size_t count);
    ///@}
};

}}}

#endif // !DSPXX_LIBSNDFILE_DISABLED
