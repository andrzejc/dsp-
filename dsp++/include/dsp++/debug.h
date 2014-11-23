/*!
 * @file dsp++/debug.h
 * @brief Debugging helpers
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_DEBUG_H_INCLUDED
#define DSP_DEBUG_H_INCLUDED
#pragma once

#include <dsp++/export.h>
#include <string>

namespace dsp { namespace dbg {

DSPXX_API void dump_str(std::string& str, const float* vec, size_t len);
DSPXX_API void dump_str(std::string& str, const double* vec, size_t len);

DSPXX_API void dump_csv(const char* path, const float* vec, size_t len);
DSPXX_API void dump_csv(const char* path, const double* vec, size_t len);

DSPXX_API void clipbrd_copy(const float* vec, size_t len);
DSPXX_API void clipbrd_copy(const double* vec, size_t len);

} }

#endif /* DSP_DEBUG_H_INCLUDED */
