#include <dsp++/platform.h>
#ifdef DSP_ARCH_FAMILY_X86

#include <dsp++/simd.h>
#include "../../simd.h"
#include "sse.h"

#ifdef _MSC_VER
# include <malloc.h> // for _aligned_malloc()/_aligned_free()
# include <intrin.h>	// for __cpuid()

# if !defined(__CLR_VER) && defined(_M_X64) && (_MSC_FULL_VER >= 160040219)
#  include <immintrin.h>  // AVX intrinsics, for _xgetbv()
# endif
#endif // _MSC_VER

#ifdef __GNUC__
# if (defined(__pic__) || defined(__APPLE__))
static __inline void __cpuid(int cpu_info[4], int info_type) {
  asm volatile (
    "mov %%ebx, %%edi                          \n"
    "cpuid                                     \n"
    "xchg %%edi, %%ebx                         \n"
    : "=a"(cpu_info[0]), "=D"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
    : "a"(info_type));
}
# else
static __inline void __cpuid(int cpu_info[4], int info_type) {
  asm volatile (
    "cpuid                                     \n"
    : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
    : "a"(info_type));
}
# endif

static __inline unsigned int xgetbv(unsigned int xcr) {
  unsigned int xcr_feature_mask;
  asm volatile (
    ".byte 0x0f, 0x01, 0xd0\n"
    : "=a"(xcr_feature_mask)
    : "c"(xcr)
    : "memory", "cc", "edx");  // edx unused.
  return xcr_feature_mask;
}
#endif // __GNUC__

#ifdef _MSC_VER
# if !defined(__CLR_VER) && defined(_M_X64) && (_MSC_FULL_VER >= 160040219)
static unsigned int xgetbv(unsigned int xcr) {
  return static_cast<unsigned int>(_xgetbv(xcr));
}
# elif !defined(__CLR_VER) && defined(_M_IX86)
__declspec(naked) __declspec(align(16))
static unsigned int xgetbv(unsigned int xcr) {
  __asm {
    mov        ecx, [esp + 4]    // xcr
    _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0  // xgetbv for vs2005.
    ret
  }
}
# endif
#endif// _MSC_VER

#define CPUID3_MMX_BIT (1 << 23)
#define CPUID3_SSE_BIT (1 << 25)
#define CPUID3_SSE2_BIT (1 << 26)
#define CPUID3_SSE2_CLFLUSH_BIT (1 << 19)
#define CPUID2_SSE3_BIT (1 << 0)
#define CPUID2_SSSE3_BIT (1 << 9)
#define CPUID2_FMA3_BIT (1 << 12)
#define CPUID2_SSE41_BIT (1 << 19)
#define CPUID2_SSE42_BIT (1 << 20)
#define CPUID2_XSAVE_OS_BIT (1 << 27)
#define CPUID2_AVX_BIT (1 << 28)

DSPXX_API int dsp::simd::features() {
	int regs[4];
#if 0
	__cpuid(regs, 0);
	if (regs[0] < 1)
		// cpuid level 1 not supported
		return 0;
#endif
	__cpuid(regs, 1);
	int res = 0;
	if (regs[3] & CPUID3_MMX_BIT)
		res |= feat_x86_mmx;
	if (regs[3] & CPUID3_SSE_BIT)
		res |= feat_x86_sse;
	if (regs[3] & CPUID3_SSE2_BIT)
		res |= feat_x86_sse2;
	if (regs[2] & CPUID2_SSE3_BIT)
		res |= feat_x86_sse3;
	if (regs[2] & CPUID2_SSSE3_BIT)
		res |= feat_x86_ssse3;
	if (regs[2] & CPUID2_FMA3_BIT)
		res |= feat_x86_fma3;
	if (regs[2] & CPUID2_SSE41_BIT)
		res |= feat_x86_sse41;
	if (regs[2] & CPUID2_SSE42_BIT)
		res |= feat_x86_sse42;

	// AVX requires OS support which is detected by testing whether OS enabled XSAVE bit and xgetbv magic
#define AVX_MASK (CPUID2_AVX_BIT | CPUID2_XSAVE_OS_BIT)
#ifndef _XCR_XFEATURE_ENABLED_MASK
# define _XCR_XFEATURE_ENABLED_MASK 0
#endif

	if (AVX_MASK == (regs[2] & AVX_MASK)) {
	    unsigned int xcr = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
		if (xcr & 0x6) {
			res |= feat_x86_avx;

			__cpuid(regs, 0x80000000);
			if ((unsigned)regs[0] >=  0x80000001) {
				__cpuid(regs, 0x80000001);
				// AMD XOP & FMA4 require AVX support
				if (regs[2] & 0x00000800)
					res |= feat_x86_xop;
				if (regs[2] & 0x00010000)
					res |= feat_x86_fma4;
			}
		}
	}
	return res;
}

DSPXX_API size_t dsp::simd::alignment() {
	return (DSP_SIMD_FEATURES & dsp::simd::feat_x86_avx ? 32 : 16);
}

DSPXX_API int dsp::simd::architecture() {
#ifdef DSP_ARCH_X86_64
	return dsp::simd::arch_x86_64;
#else
	return dsp::simd::arch_x86;
#endif
}

DSPXX_API void* dsp::simd::aligned_alloc(size_t size)
{
	static const size_t alignment_ = dsp::simd::alignment();
	void* ptr;
#if (_MSC_VER)
	ptr = _aligned_malloc(size, alignment_);
#else
	if (DSP_SIMD_FEATURES & feat_x86_sse)
		ptr = dsp::simd::detail::x86_sse_alloc(size, alignment_);
	else
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
	if (DSP_SIMD_FEATURES & feat_x86_sse)
		dsp::simd::detail::x86_sse_free(ptr);
	else
		dsp::simd::detail::generic_aligned_free(ptr);
#endif
}

#endif // DSP_ARCH_FAMILY_X86

