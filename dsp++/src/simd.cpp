#include <dsp++/simd.h>
#include <dsp++/platform.h>

#include <cstdlib> // for posix_memalign()/aligned_alloc()/malloc()/free()
#include <cassert>

#include "simd.h"

using namespace dsp::simd;

void* dsp::simd::detail::generic_aligned_alloc(size_t size)
{
	static const size_t alignment_ = dsp::simd::alignment();
	void* ptr;
#if (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600)
	if (0 != ::posix_memalign(&ptr, size, alignment_))
		ptr = NULL;
#elif (_ISOC11_SOURCE)
	ptr = ::aligned_alloc(alignment_, size);
#else
    ptr = std::malloc(size + alignment_);
    if (NULL == ptr)
        return NULL;
    long diff 			= ((~(long)ptr)&(alignment_ - 1)) + 1;
    ptr               	= (char *)ptr + diff;
    ((char *)ptr)[-1] = (char)diff;
#endif
    if (NULL == ptr && 0 == size)
    	ptr = dsp::simd::aligned_alloc(1);
    return ptr;
}

void dsp::simd::detail::generic_aligned_free(void* ptr)
{
#if (_POSIX_C_SOURCE >= 200112L) || (_XOPEN_SOURCE >= 600) || (_ISOC11_SOURCE)
	std::free(ptr);
#else
    if (NULL == ptr)
    	return;

#ifndef NDEBUG
	static const size_t alignment_ = dsp::simd::alignment();
#endif

	int v = ((char *)ptr)[-1];
	assert(v > 0 && v <= (int)alignment_);
	std::free((char *)ptr - v);
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

