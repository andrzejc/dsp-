#include <dsp++/simd.h>
#include <dsp++/platform.h>

#include <malloc.h> // for _aligned_malloc()/_aligned_free()/memalign()
#include <cstdlib> // for posix_memalign()
#include <cassert>

using namespace dsp::simd;

namespace {

#if defined(DSP_ARCH_FAMILY_X86)

#if defined(__GNUC__)
#if (defined(__pic__) || defined(__APPLE__))
static __inline void __cpuid(int cpu_info[4], int info_type) {
  asm volatile (
    "mov %%ebx, %%edi                          \n"
    "cpuid                                     \n"
    "xchg %%edi, %%ebx                         \n"
    : "=a"(cpu_info[0]), "=D"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
    : "a"(info_type));
}
#else
static __inline void __cpuid(int cpu_info[4], int info_type) {
  asm volatile (
    "cpuid                                     \n"
    : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
    : "a"(info_type));
}
#endif
#endif // defined(__GNUC__)

#ifdef _MSC_VER
#include <intrin.h>

#if !defined(__CLR_VER) && defined(_M_X64) && defined(_MSC_VER) && (_MSC_FULL_VER >= 160040219)
#include <immintrin.h>  // For _xgetbv()
#endif
#endif // _MSC_VER


#if !defined(__CLR_VER) && defined(_M_X64) && defined(_MSC_VER) && (_MSC_FULL_VER >= 160040219)
static unsigned int xgetbv(unsigned int xcr) {
  return static_cast<unsigned int>(_xgetbv(xcr));
}
#elif !defined(__CLR_VER) && defined(_M_IX86) && defined(_MSC_VER)
__declspec(naked) __declspec(align(16))
static unsigned int xgetbv(unsigned int xcr) {
  __asm {
    mov        ecx, [esp + 4]    // xcr
    _asm _emit 0x0f _asm _emit 0x01 _asm _emit 0xd0  // xgetbv for vs2005.
    ret
  }
}
#else
static unsigned int xgetbv(unsigned int xcr) {
  unsigned int xcr_feature_mask;
  asm volatile (
    ".byte 0x0f, 0x01, 0xd0\n"
    : "=a"(xcr_feature_mask)
    : "c"(xcr)
    : "memory", "cc", "edx");  // edx unused.
  return xcr_feature_mask;
}
#endif


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

static int do_get_features() {
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
		res |= feat_x86_sse4_1;
	if (regs[2] & CPUID2_SSE42_BIT)
		res |= feat_x86_sse4_2;

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

static const int features_ = do_get_features();
static const size_t alignment_ = ((features_ & feat_x86_avx) ? 32 : 16);
#ifdef DSP_ARCH_X86_64
static const int architecture_ = dsp::simd::arch_x86_64;
#else
static const int architecture_ = dsp::simd::arch_x86;
#endif

#elif defined(DSP_ARCH_FAMILY_PPC)

static const size_t alignment_ = 16;
#ifdef DSP_ARCH_PPC64
static const int architecture_ = dsp::simd::arch_ppc64;
#else
static const int architecture_ = dsp::simd::arch_ppc;
#endif

// TODO add features/architecture/alignment for other platforms
#else
static const int features_ = 0;
static const size_t alignment_ = 1;
static const int architecture_ = dsp::simd::arch_unknown;
#endif

} // end of anonymous namespace 

DSPXX_API int dsp::simd::architecture()
{
	return architecture_;
}

DSPXX_API int dsp::simd::features()
{
	return features_;
}

DSPXX_API size_t dsp::simd::alignment()
{
	return alignment_;
}

DSPXX_API void* dsp::simd::aligned_alloc(size_t size)
{
	void* ptr;
#if (_MSC_VER)
	ptr = _aligned_malloc(size, alignment_);
#elif (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)
	if (0 != posix_memalign(&ptr, alignment_, size))
		ptr = NULL;
#elif (_ISOC11_SOURCE)
	ptr = ::aligned_alloc(alignment_, size);
#else
    ptr = malloc(size + alignment_);
    if (NULL == ptr)
        return NULL;
    long diff 			= ((~(long)ptr)&(alignment_ - 1)) + 1;
    ptr               	= (char *)ptr + diff;
    ((char *)ptr)[-1] = diff;
#endif
    if (NULL == ptr && 0 == size)
    	ptr = dsp::simd::aligned_alloc(1);
    return ptr;
}

DSPXX_API void dsp::simd::aligned_free(void* ptr)
{
#if (_MSC_VER)
	_aligned_free(ptr);
#elif (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600) || (_ISOC11_SOURCE)
	free(ptr);
#else
    if (NULL == ptr)
    	return;

	int v = ((char *)ptr)[-1];
	assert(v > 0 && v <= (int)alignment_);
	free((char *)ptr - v);
#endif
}

DSPXX_API size_t dsp::simd::aligned_count(size_t count, size_t element_size)
{
	assert(element_size <= alignment_ || 0 == (element_size % alignment_));
	if (element_size > alignment_)
		return count;

	assert(0 == (alignment_ % element_size));
	size_t sz = count * element_size;
	if (sz <= alignment_)
		return alignment_ / element_size;
	
	if (0 == (sz % alignment_))
		return count;

	return (alignment_ / element_size) * (sz / alignment_ + 1);
}

