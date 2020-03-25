#include <dsp++/platform.h>

#ifdef DSPXX_ARCH_FAMILY_ARM

#include <dsp++/simd.h>
#include "../../simd.h"

#ifdef _MSC_VER
# include <malloc.h> // for _aligned_malloc()/_aligned_free()
# include <intrin.h>	// for __cpuid() && Intel or ARM intrinsics
#endif // _MSC_VER

#ifdef __GNUC__
#endif // __GNUC__

#ifdef _MSC_VER
#endif// _MSC_VER

DSPXX_API unsigned dsp::simd::features() {
	unsigned res = 0;
	return res;
}

DSPXX_API size_t dsp::simd::alignment() {
	// XXX 8 byte alignment is used by ARM-EABI
	return 8;
}

DSPXX_API unsigned dsp::simd::architecture() {
	unsigned arch = 0;
#ifdef DSPXX_ARCH_ARM
	arch |= dsp::simd::arch::arm;
#endif
#ifdef DSPXX_ARCH_ARM
	//arch |= dsp::simd::arch::thumb;
#endif
#ifdef DSPXX_ARCH_ARM
	arch |= dsp::simd::arch::arm64;
#endif
	return arch;
}

DSPXX_API void* dsp::simd::aligned_alloc(size_t size)
{
	static const size_t alignment_ = dsp::simd::alignment();
	void* ptr;
#if (_MSC_VER)
	ptr = _aligned_malloc(size, alignment_);
#else
	ptr = dsp::simd::detail::generic_aligned_alloc(size);
#endif
	if (NULL == ptr && 0 == size)
		ptr = dsp::simd::aligned_alloc(1);
	return ptr;
}

DSPXX_API void dsp::simd::aligned_free(void* ptr)
{
#if (_MSC_VER)
	_aligned_free(ptr);
#else
	dsp::simd::detail::generic_aligned_free(ptr);
#endif
}

#endif // DSPXX_ARCH_FAMILY_ARM

