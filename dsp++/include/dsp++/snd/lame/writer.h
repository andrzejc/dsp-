/*!
 * @file dsp++/snd/lame/writer.h
 * @brief Declaration of class dsp::snd::lame::writer.
 */
#pragma once
#include <dsp++/config.h>

#ifndef DSPXX_LAME_DISABLED

#include <dsp++/types.h>
#include <dsp++/snd/io.h>
#include <dsp++/snd/format.h>

#include <memory>

namespace dsp { namespace snd { namespace lame {

//! @brief Simple interface for writing sound files (uses libmp3lame as a back-end).
class DSPXX_API writer: public snd::writer {
    struct impl;
    std::unique_ptr<impl> impl_;

public:
    /*!
     * @brief Constructor.
     * To open the actual stream use one of the open() methods.
     */
    writer();

    ~writer();

    void open(const char* path, const file_format& format);
    void open(std::unique_ptr<byte_stream>, const file_format& format);

    void close();
    bool is_open() const;
    const file_format& format() const override;
    size_t frame_count() const override;
    unsigned sample_rate() const;
    unsigned channel_count() const;
    bool seekable() const override { return false; }
    size_t seek(ssize_t offset, int whence) override;
    bool supports_properties() const override { return true; }
    optional<string> property(string_view prop) override;
    void set_property(string_view metadata, string_view value) override;
    void commit() override;

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
     * @throw lame::error is thrown if error is reported by underlying libmp3lame.
     */
    size_t write_frames(const float* buf, size_t count) override;
    size_t write_frames(const short* buf, size_t count) override;
    size_t write_frames(const int* buf, size_t count) override;
    size_t write_frames(const double* buf, size_t count) override;
    ///@}
};

}}}

#endif // !DSPXX_LAME_DISABLED
