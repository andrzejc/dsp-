/// @file dsp++/snd/io_error.h
/// @brief Definition of @c io_error exception class.
#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>
#include <stdexcept>

namespace dsp { namespace snd {

/*!
 * @brief Sound input/output error.
 */
class DSPXX_API io_error: public std::runtime_error {
public:
    explicit io_error(const string& msg):
        runtime_error(msg)
    {}
};

}}
