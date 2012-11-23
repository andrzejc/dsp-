/*!
 * @file dsp++/snd/io_error.h
 * @brief Definition of @c io_error exception class.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_IO_ERROR_H_INCLUDED
#define DSP_IO_ERROR_H_INCLUDED

#include <dsp++/export.h>
#include <stdexcept>

namespace dsp { namespace snd {

/*!
 * @brief Sound input/output error.
 */
class DSPXX_API io_error: public std::runtime_error {
public:
	io_error(const std::string& msg)
	 :	runtime_error(msg)
	{
	}
private:
};

} }

#endif /* DSP_IO_ERROR_H_INCLUDED */
