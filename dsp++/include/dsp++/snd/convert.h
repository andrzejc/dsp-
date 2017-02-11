/*!
 * @file dsp++/snd/convert.h
 * @brief Sample buffer conversion routines
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_CONVERT_H_INCLUDED
#define DSP_SND_CONVERT_H_INCLUDED

#include <dsp++/export.h>

namespace dsp { namespace snd {

namespace buf {
struct info;
struct layout;
}

struct sample_layout;

DSPXX_API void convert_samples(const sample_layout& sl_in, unsigned sample_stride_in, const void* in,
	const sample_layout& sl_out, unsigned sample_stride_out, void* out, unsigned length);

DSPXX_API void convert_samples(const sample_layout& sl_in, const buf::layout& bl_in, const void* in,
	const sample_layout& sl_out, const buf::layout& bl_out, void* out, unsigned length, unsigned channels);

}}

#endif /* DSP_SND_CONVERT_H_INCLUDED */
