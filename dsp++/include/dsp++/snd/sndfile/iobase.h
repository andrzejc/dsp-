/*!
 * @file dsp++/snd/sndfile/iobase.h
 * @brief C++ interface to audio file handling with libsndfile - common part.
 */
#pragma once
#include <dsp++/config.h>

#if !DSPXX_LIBSNDFILE_DISABLED
#include <dsp++/export.h>
#include <dsp++/types.h>
#include <dsp++/snd/io.h>

#include <cstdio> // for FILE
#include <memory>

typedef struct SNDFILE_tag SNDFILE; //!< Forward declaration from libsndfile.

namespace dsp { namespace snd {

class file_format;

//! @brief Glue to libsndfile APIs.
namespace sndfile {

namespace detail {
/*!
 * @brief Base class for reader and writer, encapsulating most of
 * the shared code interfacing with libsndfile.
 */
class DSPXX_API iobase: virtual snd::iobase {
    struct impl;
    std::unique_ptr<impl> impl_;
    friend impl;

protected:
    enum class mode {
        read, write, rw
    };
    /*!
     * @brief Constructor for use by the subclasses.
     * @param read open mode (read/write).
     */
    explicit iobase(mode m);
    /*!
     * @brief Throw last libsndfile error as sndfile::error.
     * @throw error always.
     */
    void throw_last_error();

public:
    virtual ~iobase();

    //! @name Opening and closing the I/O stream/file.
    ///@{
    /*!
     * @brief Open file specified by given path.
     * @param path file path.
     * @param fmt optional format object used as hint for reader when the file type
     * doesn't include the necessary information (e.g. when reading raw files) and
     * mandatory when opening file for writing.
     * @param native_info alternatively to dsp::snd::format, use native (SF_INFO)
     * object to pass file format information.
     * @throw sndfile::error when libsndfile is unable to open the file.
     */
    void open(const char* path, file_format* fmt = nullptr, void* native_info = nullptr);

#ifdef _WIN32
    /*!
     * @brief Open file specified by given path.
     * @param path file path.
     * @param fmt optional format object used as hint for reader when the file type
     * doesn't include the necessary information (e.g. when reading raw files) and
     * mandatory when opening file for writing.
     * @param native_info alternatively to dsp::snd::format, use native (SF_INFO)
     * object to pass file format information.
     * @throw sndfile::error when libsndfile is unable to open the file.
     */
    void open(const wchar_t* path, file_format* fmt = nullptr, void* native_info = nullptr);
#endif // _WIN32

    /*!
     * @brief Open specified stdio stream.
     * @param f FILE struct to open.
     * @param own_file indicate whether fclose() should be called on the
     * specified file when iobase::close() is called (or destructor is invoked).
     * @param fmt optional file_format object used as hint for reader when the file type
     * doesn't include the necessary information (e.g. when reading raw files) and
     * mandatory when opening file for writing.
     * @param native_info alternatively to dsp::snd::format, use native (SF_INFO)
     * object to pass file format information.
     * @throw sndfile::error when libsndfile is unable to open the file.
     */
    void open(std::FILE* f, bool own_file, file_format* fmt = nullptr, void* native_info = nullptr);
    /*!
     * @brief Open specified file descriptor.
     * @param fd file descriptor to open.
     * @param own_fd indicate whether file close should be called on the
     * specified fd when iobase::close() is called (or destructor is invoked).
     * @param fmt optional file_format object used as hint for reader when the file type
     * doesn't include the necessary information (e.g. when reading raw files) and
     * mandatory when opening file for writing.
     * @param native_info alternatively to dsp::snd::file_format, use native (SF_INFO)
     * object to pass file format information.
     * @throw sndfile::error when libsndfile is unable to open the file.
     */
    void open(int fd, bool own_fd, file_format* fmt = nullptr, void* native_info = nullptr);
    /*!
     * @brief Open specified abstract io stream.
     * @param in io stream abstraction.
     * @param own_io indicate whether dsp::snd::io object should be deleted when
     * iobase::close() is called (or destructor is invoked).
     * @param fmt optional file_format object used as hint for reader when the file type
     * doesn't include the necessary information (e.g. when reading raw files) and
     * mandatory when opening file for writing.
     * @param native_info alternatively to dsp::snd::file_format, use native (SF_INFO)
     * object to pass file format information.
     * @throw sndfile::error when libsndfile is unable to open the file.
     */
    void open(std::unique_ptr<byte_stream> in, file_format* fmt = nullptr, void* native_info = nullptr);

    //! @brief Close the audio file, free the resources.
    void close();

    bool is_open() const;
    ///@}

    //! @name Audio stream properties.
    ///@{
    /*!
     * @brief Query sample rate of open file.
     * @return sample rate (AKA sampling frequency) of the audio stream.
     */
    unsigned sample_rate() const;
    /*!
     * @brief Query number of channels of open file.
     * @return number of channels.
     */
    unsigned channel_count() const;
    /*!
     * @brief Query number of frames stored within the audio file.
     * @return number of frames (file length expressed in sampling periods).
     */
    size_t frame_count() const override;
    ///@}

    /*!
     * @brief Query whether the underlying I/O model and file format support seek() function.
     * @return true if seeking is supported.
     */
    bool seekable() const override;
    /*!
     * @brief Seek the stream to the specified position (expressed in frames).
     * @param frames_offset position in frames to seek to.
     * @param whence position relative to (fseek() semantics).
     * @return new position in stream.
     * @throw sndfile::error on fail.
     */
    size_t seek(ssize_t frames_offset, int whence) override;

    //! @name Lower-level API.
    ///@{
    /*!
     * @brief Access to raw libsndfile implementation.
     * @return SNDFILE struct pointer, NULL if file is not open.
     */
    SNDFILE* handle();
    /*!
     * @brief Issue the special libsndfile command.
     * @param cmd one of libsndfile SFC_* command ids.
     * @param data command-specific data.
     * @param datasize size of data buffer.
     */
    int command(int cmd, void* data, int datasize);
    ///@}

    const file_format& format() const override;

    bool supports_properties() const override;
    optional<string> property(string_view metadata) override;

    void set_property(string_view metadata, string_view value);

    void commit();
};
}

}}}

#endif // !DSPXX_LIBSNDFILE_DISABLED
