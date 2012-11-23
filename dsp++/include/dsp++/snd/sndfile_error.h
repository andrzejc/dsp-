/*!
 * @file dsp++/snd/sndfile_error.h
 * @brief Definition of @c sndfile_error exception class.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_SNDFILE_ERROR_H_INCLUDED
#define DSP_SNDFILE_ERROR_H_INCLUDED

#include <dsp++/io_error.h>

namespace dsp { namespace snd {

/*!
 * @brief Wrapper for libsndfile error codes (@c SF_ERR_* constants from <sndfile.h>).
 * @see http://www.mega-nerd.com/libsndfile/
 * @see @c sf_error()
 * @see @c sf_error_number()
 */
class DSPXX_API sndfile_error: public io_error {
public:
	/*!
	 * @brief Construct exception based on given error code and text message.
	 * @param code libsndfile error code.
	 * @param msg textual representation of the error obtained through @c sf_error_number().
	 */
	sndfile_error(int code, const std::string& msg)
	 :	io_error(msg)
	 ,	code_(code)
	{
	}
	/*!
	 * @brief Query error code.
	 * @return libsndfile error code.
	 */
	int code() const {return code_;}

private:
	int code_;
};

} }

#endif /* DSP_SNDFILE_ERROR_H_INCLUDED */
