#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>

#include <absl/types/optional.h>

namespace dsp { namespace snd {

struct DSPXX_API iobase {
    virtual ~iobase() = default;

    virtual bool supports_properties() const = 0;
    virtual absl::optional<string> property(string_view prop) = 0;

    virtual bool seekable() const = 0;
    virtual size_t seek(ssize_t frames, int whence) = 0;
};

struct DSPXX_API reader: virtual iobase {
	/*!
	 * @name Frame-based input.
	 */
    ///@{
    /*!
     * @brief Read up to @p count frames from sound file into @p buf.
     * @param buf buffer with with enough space to hold count * channel_count() samples.
     * @param count number of frames to read.
     * @return number of frames read (0 if EOF is reached).
     * @throw std::exception is thrown if error is reported by implementation.
     */
    virtual size_t read_frames(float* buf, size_t count) = 0;
    virtual size_t read_frames(short* buf, size_t count) = 0;
    virtual size_t read_frames(int* buf, size_t count) = 0;
    virtual size_t read_frames(double* buf, size_t count) = 0;
    ///@}
};

struct DSPXX_API writer: virtual iobase {
    /*!
     * @name Frame-based output.
     */
    ///@{
    /*!
     * @brief Write @p count frames from @p buf to the sound file.
     * @param buf buffer holding count * channel_count() samples.
     * @param count number of frames to write.
     * @return number of frames written.
     * @ingroup Frame
     * @throw std::exception is thrown if error is reported by implementation.
     */
    virtual size_t write_frames(const float* buf, size_t count) = 0;
    virtual size_t write_frames(const short* buf, size_t count) = 0;
    virtual size_t write_frames(const int* buf, size_t count) = 0;
    virtual size_t write_frames(const double* buf, size_t count) = 0;
    ///@}
    virtual void commit() = 0;

    virtual void set_property(string_view metadata, string_view value) = 0;
};

struct DSPXX_API file: public virtual reader, public virtual writer {
};

}}