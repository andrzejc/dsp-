#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>

namespace dsp { namespace snd {

/*!
 * @brief Core I/O abstraction used for reading/writing sound file bytes..
 * Closely mimics stdio functions.
 * @see dsp::snd::stdio.
 */
class DSPXX_API byte_stream {
public:
    /*!
     * @brief Query file size.
     * @return file size.
     * @throw on I/O error.
     */
    virtual size_t size();
    /*!
     * @brief Seek within the abstracted stream.
     * @param offset seek position.
     * @param whence seek relative to (using the same constants as fseek()).
     * @return new position in stream or -1 on error.
     * @throw on I/O error.
     */
    virtual size_t seek(ssize_t offset, int whence) = 0;
    /*!
     * @brief Read size bytes from the stream into the buf.
     * @param buf buffer to read data to.
     * @param size number of bytes to read.
     * @return number of bytes read or 0 on EOF.
     * @throw on I/O error.
     */
    virtual size_t read(void* buf, size_t size) = 0;
    /*!
     * @brief Write size bytes from buf to the stream.
     * @param buf buffer holding the data to write.
     * @param size number of bytes to write.
     * @return number of bytes written.
     * @throw on I/O error.
     */
    virtual size_t write(const void* buf, size_t size) = 0;
    /*!
     * @brief Query actual stream position.
     * @return stream position in bytes.
     * @throw on I/O error.
     */
    virtual size_t position();

    virtual void flush() = 0;

    /*!
     * @brief This is interface class.
     */
    virtual ~byte_stream() = default;

    struct position_saver {
        byte_stream& stream;
        const size_t pos;

        explicit position_saver(byte_stream& stream);
        ~position_saver() noexcept(false);
    };
};

/*!
 * @brief dsp::snd::byte_stream subclass implemented with Standard C Library std::FILE* interface.
 * Makes it possible to use standard input/output pipes for reading/writing
 * audio files.
 */
class DSPXX_API stdio_stream: public byte_stream {
    std::FILE* file_;
    bool own_file_;

public:
    /*!
     * @brief Create stdio_stream object with specified std::FILE pointer.
     * @param file file open with std::fopen() or standard input/output
     * stream.
     * @param own_file if true, file will be closed with std::fclose()
     * in this object's destructor. Standard input/output streams should
     * be left open.
     */
    explicit constexpr stdio_stream(std::FILE* file, bool own_file = true):
        file_(file),
        own_file_(own_file)
    {}

    /// @brief Equivalent of @c stdio_stream{std::fopen(path, mode), true} with error checking.
    explicit stdio_stream(const char* path, const char* mode);
#ifdef _WIN32
    /// @brief Equivalent of @c stdio_stream{_wfopen(path, mode), true} with error checking.
    explicit stdio_stream(const wchar_t* path, const wchar_t* mode);
#endif
    /// @brief Equivalent of @c stdio_stream{fdopen(fd, mode), true} with error checking.
    explicit stdio_stream(int fd, const char* mode);

    explicit operator bool() const noexcept {
        return file_ != nullptr;
    }
    std::FILE* file() const noexcept {
        return file_;
    }

    size_t size() override;
    size_t seek(ssize_t offset, int whence) override;
    size_t read(void* buf, size_t size) override;
    size_t write(const void* buf, size_t size) override;
    size_t position() override;
    void flush() override;
    /*!
     * @brief Closes the associated FILE stream with a call to fclose()
     * if it is owned by this stdio_stream object.
     */
    ~stdio_stream();
};

/*!
 * @brief dsp::snd::byte_stream subclass implemented with POSIX file descriptor.
 * Makes it possible to use pipes or sockets and other low-level I/O like mmap.
 */
class DSPXX_API fildes_stream: public byte_stream {
    int fd_ = -1;
    bool own_fd_ = false;

public:
    /*!
     * @brief Create stdio object with specified FILE pointer.
     * @param file file open with std::fopen() or standard input/output
     * stream.
     * @param own_file if true, file will be closed with std::fclose()
     * in this object's destructor. Standard input/output streams should
     * be left open.
     */
    explicit constexpr fildes_stream(int fd, bool own_fd = true):
        fd_{fd},
        own_fd_{own_fd}
    {}

    /// @brief Equivalent of @c fildes_stream{open(path, flags, mode?), true} with error checking.
    explicit fildes_stream(const char* path, int flags, int mode = -1);
#ifdef _WIN32
    explicit fildes_stream(const wchar_t* path, int flags, int mode = -1);
#endif

    explicit operator bool() const noexcept {
        return fd_ >= 0;
    }
    int fildes() const noexcept {
        return fd_;
    }

    size_t seek(ssize_t offset, int whence) override;
    size_t read(void* buf, size_t size) override;
    size_t write(const void* buf, size_t size) override;
    void flush() override;
    /*!
     * @brief Closes the associated file descriptor call to close()
     * if it is owned by this fildes_stream object.
     */
    ~fildes_stream();
};

class file_format;

struct DSPXX_API iobase {
    virtual ~iobase() = default;

    virtual const file_format& format() const = 0;
    virtual size_t frame_count() const = 0;

    virtual bool supports_properties() const = 0;
    virtual optional<string> property(string_view prop) = 0;

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