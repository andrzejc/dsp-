#include <dsp++/platform.h>

#ifdef DSP_ARCH_FAMILY_X86

#include <dsp++/simd.h>
#include "sse.h"
#include "sse_utils.h"

#include <pmmintrin.h>

//! @brief Dot product using SSE3 instruction set.
SSE3_SUM_FVVS(dsp::simd::detail::x86_sse3_dotf, mul_ps)

#endif // DSP_ARCH_FAMILY_X86
