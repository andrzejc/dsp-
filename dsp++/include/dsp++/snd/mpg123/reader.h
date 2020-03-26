/*!
 * @file dsp++/snd/mpg123/reader.h
 * @brief Declaration of class dsp::snd::mpg123::reader.
 */
#pragma once
#include <dsp++/config.h>

#if !DSPXX_MPG123_DISABLED

#include <dsp++/snd/io.h>
#include <dsp++/snd/format.h>

#include <absl/types/optional.h>

#include <memory>

namespace dsp { namespace snd { namespace mpg123 {

//! @brief Simple interface for reading sound files (uses libsndfile as a back-end).
// TODO make detail::iobase private
class DSPXX_API reader: public snd::reader {
    struct impl;
    std::unique_ptr<impl> impl_;

public:
    /*!
     * @brief Constructor.
     * To open the actual stream use one of the open() methods.
     */
    reader();

    ~reader();

    void open(const char* path, file_format* format = nullptr);
    void open(int fd, bool own_fd, file_format* format = nullptr);

    void close();
    bool is_open() const;
    const file_format& format() const override;
    size_t frame_count() const override;
    unsigned sample_rate() const;
    unsigned channel_count() const;
    bool seekable() const override { return true; }
    size_t seek(ssize_t offset, int whence) override;
    bool supports_properties() const override { return true; }
    absl::optional<string> property(string_view prop) override;

    /*!
     * @name Frame-based input.
     */
    ///@{
    /*!
     * @brief Read up to @p count frames from sound file into @p buf.
     * @param buf buffer with with enough space to hold count * channel_count() samples.
     * @param count number of frames to read.
     * @return number of frames read (0 if EOF is reached).
     * @throw mpg123::error or is thrown if error is reported by underlying mpg123.
     */
    size_t read_frames(float* buf, size_t count) override;
    size_t read_frames(short* buf, size_t count) override;
    size_t read_frames(int* buf, size_t count) override;
    size_t read_frames(double* buf, size_t count) override;
    ///@}
};

}}}

#endif // !DSPXX_MPG123_DISABLED
