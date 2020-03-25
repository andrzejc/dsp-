#include <dsp++/simd.h>
#include <dsp++/platform.h>

#include <cstdlib> // for posix_memalign()/aligned_alloc()/malloc()/free()
#include <cassert>

#if defined(DSPXX_OS_ANDROID)
#include <malloc.h>
#endif // DSPXX_OS_ANDROID

#include "simd.h"

using namespace dsp::simd;

void* dsp::simd::detail::generic_aligned_alloc(size_t size)
{
	static const size_t alignment_ = dsp::simd::alignment();
	void* ptr;
	while (true)
	{
#if defined(DSPXX_OS_ANDROID)
		ptr = ::memalign(alignment_, size);
#elif (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)
		if (0 != ::posix_memalign(&ptr, size, alignment_))
			ptr = NULL;
#elif (_ISOC11_SOURCE)
		ptr = ::aligned_alloc(alignment_, size);
#else
		ptr = std::malloc(size + alignment_);
		if (NULL == ptr)
			return NULL;
		intptr_t diff = ((~reinterpret_cast<intptr_t>(ptr)) & (static_cast<intptr_t>(alignment_) - 1)) + 1;
		ptr = static_cast<char*>(ptr) + diff;
		static_cast<char*>(ptr)[-1] = static_cast<char>(diff);
#endif
		if (NULL == ptr && 0 == size)
			size = 1;
		else
			break;
	}
    return ptr;
}

void dsp::simd::detail::generic_aligned_free(void* ptr)
{
#if (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600) || (_ISOC11_SOURCE) || defined(DSPXX_OS_ANDROID)
	std::free(ptr);
#else
    if (NULL == ptr)
    	return;

#ifndef NDEBUG
	static const int alignment_ = static_cast<int>(dsp::simd::alignment());
#endif
	int v = static_cast<char*>(ptr)[-1];
	assert(v > 0 && v <= alignment_);
	std::free(static_cast<char*>(ptr) - v);
#endif
}

size_t dsp::simd::aligned_count(size_t count, size_t element_size)
{
	static const size_t alignment_ = alignment();
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

