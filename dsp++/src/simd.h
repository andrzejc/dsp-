#ifndef DSP_INTERNAL_SIMD_H_INCLUDED
#define DSP_INTERNAL_SIMD_H_INCLUDED
#pragma once

namespace dsp { namespace simd {namespace detail {

void* generic_aligned_alloc(size_t size);
void generic_aligned_free(void* p);

} } }

#endif /* DSP_INTERNAL_SIMD_H_INCLUDED */
