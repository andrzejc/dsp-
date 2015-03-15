/**
 * @file dsp++/simd.h
 * @brief Support for SIMD instructions
 */
#ifndef DSP_SIMD_H_INCLUDED
#define DSP_SIMD_H_INCLUDED
#pragma once

#include <dsp++/export.h>
#include <dsp++/platform.h>
#include <dsp++/buffer_traits.h>

#include <cstddef> // for size_t
#include <limits>

namespace dsp { namespace simd {

	//! @brief Processor architecture flags, not really useful in runtime - use DSP_ARCH_XXX macros in {@link dsp++/platform.h} instead.
	namespace arch { enum spec {
		unknown = 			0x0000,		//!<
		x86 = 				0x0001,   	//!< 32-bit x86 (IA32)
		x86_64 = 			0x0002, 	//!< 64-bit x86 (x86-64 aka AMD64)
		ppc =				0x0004,		//!< PowerPC
		ppc64 = 			0x0008,
		// TODO add flags for other architectures, esp. ARM

		arm =				0x0010,		//!< 32-bit ARM @todo Consider adding ARM64 and thumb
		arm64 =				0x0020,		//!< 64-bit ARM lika ARMv8-A and later
	}; }

	//! @brief Query the architecture of the runtime environment (for which the code was compiled, not the actual processor).
	//! @note If the code is compiled for x86 (32 bit executable) and is running on x86-64 processor, this will return {@link arch::x86}.
	//! This is intentional, as you can't use 64-bit instructions in 32-bit executable anyway.
	//! @return Combination of {@link arch} flags describing processor architecture.
	DSPXX_API unsigned architecture();

	//! @brief SIMD feature flags.
	//! @note For various architectures the same bits may have different meaning. It's by design, as code sections for different architectures
	//! normally will have to be\#ifdef'ed (you'll get compile time errors otherwise).
	//! @todo Add flags for other architectures when needed.
	namespace feat {enum spec {
		x86_mmx = 			0x00000001,	//!< MMX instruction set @see http://en.wikipedia.org/wiki/MMX_(instruction_set)
		x86_sse = 			0x00000002,	//!< SSE instruction set (Katmai new instructions) @see http://en.wikipedia.org/wiki/Streaming_SIMD_Extensions
		x86_sse2 =			0x00000004, //!< SSE2 instruction set @see http://en.wikipedia.org/wiki/SSE2
		x86_sse3 = 			0x00000008, //!< SSE3 instruction set (Prescott new instructions) @see http://en.wikipedia.org/wiki/Prescott_New_Instructions
		x86_ssse3 = 		0x00000010, //!< Supplemental SSE3 instructions @see http://en.wikipedia.org/wiki/SSSE3
		x86_fma3 = 			0x00000020, //!< FMA3 instruction set @see http://en.wikipedia.org/wiki/FMA_instruction_set
		x86_sse41 =			0x00000040, //!< Penryn SSE 4.1 @see http://en.wikipedia.org/wiki/SSE4.1#SSE4.1
		x86_sse42 =			0x00000080, //!< Nehalem SSE 4.2 @see http://en.wikipedia.org/wiki/SSE4.2#SSE4.2
		x86_avx = 			0x00000100, //!< Advanced Vector Extensions @see http://en.wikipedia.org/wiki/Advanced_Vector_Extensions

		x86_xop = 			0x00000200,	//!< AMD XOP @see http://en.wikipedia.org/wiki/XOP_instruction_set
		x86_fma4 =			0x00000400,	//!< AMD FMA4 @see http://en.wikipedia.org/wiki/FMA4_instruction_set

		// leaving 5 bits reserved for other x86 features
		ppc_altivec =		0x00010000,	//!< PowerPC AltiVec instructions @see http://en.wikipedia.org/wiki/AltiVec @note Not detected yet!

		// and now ARM features...
		arm_neon =			0x00100000,	
		arm_vfp2 =			0x00200000,
		arm_vfp3 =			0x00400000,
		arm_vfp4 =			0x00800000,
	}; }

	//! @brief Allows testing SIMD support of the processor in the runtime, so that alternative execution paths may be used depending on available features.
	//! @return Combination of {@link feat} flags describing available SIMD features.
	DSPXX_API unsigned features();

	//! @brief Allows checking in runtime what should be memory alignment of data passed to SIMD functions.
	DSPXX_API size_t alignment();

	DSPXX_API void* aligned_alloc(size_t size);
	DSPXX_API void aligned_free(void* p);

	//! @brief Allocator which guarantees that allocated memory be properly aligned to be used with SIMD instructions.
	template<class T> class allocator
	{
	public:
		typedef T value_type;
		typedef value_type* pointer;
		typedef const value_type* const_pointer;
		typedef value_type& reference;
		typedef const value_type& const_reference;
		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		//    convert an allocator<T> to allocator<U>
		template<typename U>
		struct rebind {
			typedef allocator<U> other;
		};

		allocator() {}
		allocator(allocator const&) {}
		template <class U> allocator(allocator<U> const&) {}

		inline pointer address(reference r) { return &r; }
		inline const_pointer address(const_reference r) { return &r; }

		inline pointer allocate(size_type cnt, const T* p = 0)
		{return reinterpret_cast<pointer>(aligned_alloc(cnt * sizeof(T)));}

		inline void deallocate(pointer p, size_type)
		{aligned_free(p);}

		inline size_type max_size() const
		{return std::numeric_limits<size_type>::max() / sizeof(T);}

		inline void construct(pointer p, const T& t) {new (p) T(t);}
		inline void destroy(pointer p) {p->~T();}

		inline bool operator==(allocator const&) { return true; }
		inline bool operator!=(allocator const& a) { return !operator==(a); }
	};

	//! @brief Required number of elements in a vector of specified length and element size, to make 
	//! the one-past-last element aligned. This function is intended as a utility for allocating several
	//! aligned vectors in one allocator call.
	//! @param count number of elements in allocated vector.
	//! @param element_size size of single element.
	//! @pre element_size <= {@link alignment()}. This is tested with an assert() and will be unconditionally true for built-in types.
	//! @pre ({@link alignment()} % element_size) == 0. This is tested with an assert() and will be unconditionally true for built-in types.
	//! @return number of elements to make the subsequent element aligned.
	DSPXX_API size_t aligned_count(size_t count, size_t element_size);

	//! @brief Required number of elements in a vector of specified length, to make the one-past-last 
	//! element aligned. This function is intended as a utility for allocating several aligned vectors in one 
	//! allocator call.
	//! @param count number of elements in allocated vector.
	//! @tparam T type of vector element.
	//! @pre sizeof(T) <= {@link alignment()}
	//! @pre ({@link alignment()} % sizeof(T)) == 0
	//! @return number of elements to make the subsequent element aligned.
	template<class T>
	inline size_t aligned_count(size_t count) {return aligned_count(count, sizeof(T));}

	//! @brief Number of padding elements in a vector of specified length, to make the one-past-last 
	//! element aligned. This function is intended as a utility for allocating several aligned vectors in one 
	//! allocator call.
	//! @param count number of elements in allocated vector.
	//! @tparam T type of vector element.
	//! @pre sizeof(T) <= {@link alignment()}
	//! @pre ({@link alignment()} % sizeof(T)) == 0
	//! @return number of padding elements after count to make the subsequent element aligned.
	template<class T>
	inline size_t aligned_pad(size_t count) {return aligned_count<T>(count) - count;}

#if defined(_MSC_VER)
# define DSP_ALIGNED(x) __declspec(align(x))
#elif defined(__GNUC__)
# define DSP_ALIGNED(x) __attribute__((aligned(x)))
#endif

	template<class Elem>
	struct buffer_traits
	{
		typedef Elem value_type;
		typedef dsp::simd::allocator<Elem> allocator_type;

		static size_t alignment()
		{
			const size_t a = dsp::simd::alignment();
			return (a > DSP_ALIGNOF(Elem) ? a : DSP_ALIGNOF(Elem));
		}

		static size_t padding_size(size_t count) {return aligned_pad<Elem>(count);}
		static size_t aligned_count(size_t count) {return dsp::simd::aligned_count<Elem>(count);}

	};
} }

#ifndef DSP_SIMD_FEATURES
namespace {
// Optimize access to dsp::simd::features() by providing a local copy in each compilation unit
static inline int dsp_simd_features_local_()
{
	static const int features = dsp::simd::features();
	return features;
}
}
//! @brief Override this macro with compiler flag specifying a set of dsp::simd::feat flags to disable runtime
//! features detection and compile for a specific instruction set.
# define DSP_SIMD_FEATURES dsp_simd_features_local_()
#endif // DSP_SIMD_FEATURES

#endif /* DSP_SIMD_H_INCLUDED */
