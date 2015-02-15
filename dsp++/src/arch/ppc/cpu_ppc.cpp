#include <dsp++/platform.h>

#ifdef DSP_ARCH_FAMILY_PPC

#if defined(DSP_OS_MACOSX)
# include <sys/sysctl.h>
#elif defined(DSP_OS_FAMILY_BSD)
# include <sys/param.h>
# include <sys/sysctl.h>
# include <machine/cpu.h>
#endif
// TODO add altivec detection on PPC linux

static const size_t alignment_ = 16;

unsigned dsp::simd::features() 
{
#if defined(DSP_OS_FAMILY_BSD)
    int sels[2] = {CTL_MACHDEP, CPU_ALTIVEC};
#elif defined(DSP_OS_MACOSX)
    int sels[2] = {CTL_HW, HW_VECTORUNIT};
#endif
    int altivec = 0;
    size_t len = sizeof(altivec);
    int err = sysctl(sels, 2, &altivec, &len, NULL, 0);
    if (0 == err)
        return (0 != altivec ? dsp::simd::feat::ppc_altivec : 0);
    return 0;
}

DSPXX_API unsigned dsp::simd::architecture()
{
#ifdef DSP_ARCH_PPC64
	return arch::ppc64;
#else
	return arch::ppc;
#endif
}

DSPXX_API size_t dsp::simd::alignment()
{
	return alignment_;
}

DSPXX_API void* dsp::simd::aligned_alloc(size_t size)
{
	return dsp::simd::detail::generic_aligned_alloc(size);
}

DSPXX_API void dsp::simd::aligned_free(void* ptr)
{
	return dsp::simd::detail::generic_aligned_free(ptr);
}

#endif
