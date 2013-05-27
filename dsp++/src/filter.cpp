/*!
 * @file filter.cpp
 * @brief Optimized specializations of filter templates (using SIMD code etc.).
 */

#include <dsp++/filter.h>
#include <dsp++/vectmath.h>
#include <dsp++/simd.h>
#include <cstring>

using namespace dsp;

