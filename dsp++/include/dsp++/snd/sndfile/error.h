/*!
 * @file dsp++/snd/sndfile/error.h
 * @brief Definition of @c dsp::snd::sndfile::error exception class.
 */
#include <dsp++/snd/io_error.h>
#include <dsp++/types.h>

namespace dsp { namespace snd { namespace sndfile {

/*!
 * @brief Wrapper for libsndfile error codes (@c SF_ERR_* constants from <sndfile.h>).
 * @see http://www.mega-nerd.com/libsndfile/
 * @see sf_error()
 * @see sf_error_number()
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
