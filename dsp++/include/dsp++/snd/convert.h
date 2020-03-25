/*!
 * @file dsp++/snd/convert.h
 * @brief Sample buffer conversion routines
 * @author Andrzej Ciarkowski <andrzej.ciarkowski@gmail.com>
 */
#pragma once
#include <dsp++/export.h>

namespace dsp { namespace snd {

namespace buf {
struct info;
struct layout;
}

namespace sample {
struct layout;

DSPXX_API void convert(
    const layout& sl_in,
    unsigned sample_stride_in,
    const void* in,
    const layout& sl_out,
    unsigned sample_stride_out,
    void* out,
    unsigned length
);

DSPXX_API void convert(
    const layout& sl_in,
    const buf::layout& bl_in,
    const void* in,
    const layout& sl_out,
    const buf::layout& bl_out,
    void* out,
    unsigned length, unsigned channels
);

}

}}
