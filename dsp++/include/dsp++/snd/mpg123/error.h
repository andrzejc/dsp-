#pragma once
#include <dsp++/types.h>
#include <dsp++/snd/io_error.h>

namespace dsp { namespace snd { namespace mpg123 {

/*!
 * @brief Wrapper for mpg123 error codes (@c mpg123_errors constants from <mpg123.h>).
 * @see mpg123_errcode()
 * @see mpg123_strerror()
 */
class DSPXX_API error: public io_error {
    const int code_;

public:
    /*!
     * @brief Construct exception based on given error code and text message.
     * @param code libsndfile error code.
     * @param msg textual representation of the error obtained through @c sf_error_number().
     */
    error(int code, const string& msg):
        io_error(msg),
        code_(code)
    {}

    /*!
     * @brief Query error code.
     * @return libsndfile error code.
     */
    int code() const {
        return code_;
    }
};

}}}
