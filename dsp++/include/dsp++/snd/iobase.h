/*!
 * @file dsp++/snd/iobase.h
 * @brief C++ interface to audio file handling with libsndfile - common part.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_SND_IOBASE_H_INCLUDED
#define DSP_SND_IOBASE_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_SNDFILE_DISABLED

#include <cstdio> // for FILE

#include <dsp++/export.h>
#include <dsp++/noncopyable.h>

typedef	struct SNDFILE_tag	SNDFILE;	//!< Forward declaration from libsndfile.

namespace dsp { namespace snd {

class io;

struct base_impl;
class file_format;

/*!
 * @brief Base class for reader and writer, encapsulating most of
 * the shared code interfacing with libsndfile.
 */
class DSPXX_API iobase: boost::noncopyable
{
public:
	//! @brief Just a typedef for long long to better express the intent of
	//! passing size or offset (there's no off_t on Windows).
	typedef long long size_type;

	/*!
	 * @brief Virtual destructor just in case someone wanted to do some
	 * polymorphic magic.
	 */
	virtual ~iobase();

	/*!
	 * @name Opening and closing the I/O stream/file.
	 */
	///@{
	/*!
	 * @brief Open file specified by given path.
	 * @param path file path.
	 * @param fmt optional format object used as hint for reader when the file type
	 * doesn't include the necessary information (e.g. when reading raw files) and
	 * mandatory when opening file for writing.
	 * @param native_info alternatively to dsp::snd::format, use native (SF_INFO)
	 * object to pass file format information.
	 * @throw sndfile_error when libsndfile is unable to open the file.
	 */
	void open(const char* path, file_format* fmt = NULL, void* native_info = NULL);
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
	 * @throw sndfile_error when libsndfile is unable to open the file.
	 */
	void open(FILE* f, bool own_file, file_format* fmt = NULL, void* native_info = NULL);
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
	 * @throw sndfile_error when libsndfile is unable to open the file.
	 */
	void open(int fd, bool own_fd, file_format* fmt = NULL, void* native_info = NULL);
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
	 * @throw sndfile_error when libsndfile is unable to open the file.
	 */
	void open(io* in, bool own_io, file_format* fmt = NULL, void* native_info = NULL);
	/*!
	 * @brief Close the audio file, free the resources.
	 */
	void close();
	///@}

	/*!
	 * @name Audio stream properties.
	 */
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
	size_type frame_count() const;
	///@}

	/*!
	 * @name Lower-level API.
	 */
	///@{
	/*!
	 * @brief Query whether the underlying I/O model and file format support seek() function.
	 * @return true if seeking is supported.
	 */
	bool is_seekable() const;
	/*!
	 * @brief Seek the stream to the specified position (expressed in frames).
	 * @param frames_offset position in frames to seek to.
	 * @param whence position relative to (fseek() semantics).
	 * @return new position in stream.
	 */
	size_type seek(size_type frames_offset, int whence);
	/*!
	 * @brief Access to raw libsndfile implementation.
	 * @return SNDFILE struct pointer, NULL if file is not open.
	 */
	SNDFILE* raw_handle();
	/*!
	 * @brief Issue the special libsndfile command.
	 * @param cmd one of libsndfile SFC_* command ids.
	 * @param data command-specific data.
	 * @param datasize size of data buffer.
	 */
	int command(int cmd, void* data, int datasize);
	///@}

protected:
	/*!
	 * @brief Constructor for use by the subclasses.
	 * @param read open mode (read/write).
	 */
	explicit iobase(bool read);
	/*!
	 * @brief Throw last libsndfile error as sndfile_error.
	 * @throw sndfile_error always.
	 */
	void throw_last_error();

private:
	base_impl* impl_;
};

/*!
 * @brief C++ wrapper for libsndfile SF_VIRTUAL_IO abstraction.
 * Closely mimics stdio functions.
 * @see dsp::snd::stdio.
 */
class DSPXX_API io
{
public:
	//! @copydoc iobase::size_type
	typedef iobase::size_type size_type;
	/*!
	 * @brief Query file size.
	 * @return file size or -1 on error.
	 */
	virtual size_type size() = 0;
	/*!
	 * @brief Seek within the abstracted stream.
	 * @param offset seek position.
	 * @param whence seek relative to (using the same constants as fseek()).
	 * @return new position in stream or -1 on error.
	 */
	virtual size_type seek(size_type offset, int whence) = 0;
	/*!
	 * @brief Read size bytes from the stream into the buf.
	 * @param buf buffer to read data to.
	 * @param size number of bytes to read.
	 * @return number of bytes read or -1 on error.
	 */
	virtual size_type read(void* buf, size_t size) = 0;
	/*!
	 * @brief Write size bytes from buf to the stream.
	 * @param buf buffer holding the data to write.
	 * @param size number of bytes to write.
	 * @return number of bytes written or -1 on error.
	 */
	virtual size_type write(const void* buf, size_t size) = 0;
	/*!
	 * @brief Query actual stream position.
	 * @return stream position or -1 if stream non-seekable or error occured.
	 */
	virtual size_type position() = 0;
	/*!
	 * @brief Virtual destructor for polymorphic destruction.
	 */
	virtual ~io();
};

/*!
 * @brief dsp::snd::io subclass implemented with Standard C Library FILE* interface.
 * Makes it possible to use standard input/output pipes for reading/writing
 * audio files.
 */
class DSPXX_API stdio: public io, boost::noncopyable
{
public:
	/*!
	 * @brief Create stdio object with specified FILE pointer.
	 * @param file file open with fopen() or standard input/output
	 * stream.
	 * @param own_file if true, file will be closed with fclose()
	 * in this object's destructor. Standard input/output streams should
	 * be left open.
	 */
	stdio(FILE* file, bool own_file);

	size_type size();
	size_type seek(size_type offset, int whence);
	size_type read(void* buf, size_t size);
	size_type write(const void* buf, size_t size);
	size_type position();
	/*!
	 * @brief Closes the associated FILE stream with a call to fclose()
	 * if it is owned by this stdio object.
	 */
	~stdio();

private:
	FILE* file_;
	bool own_file_;
};

} }

#endif // !DSP_SNDFILE_DISABLED

#endif /* DSP_SND_IOBASE_H_INCLUDED */
