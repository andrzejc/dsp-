/*!
 * @file dsp++/fftw/dft.h
 * @brief DFT wrapper for C++ based on fftw3
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_FFTW_ALLOCATOR_H_INCLUDED
#define DSP_FFTW_ALLOCATOR_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_FFTW_DISABLED

#include <cstddef>
#include <limits>
#include <dsp++/export.h>

namespace dsp { namespace dft { namespace fftw {

/*!
 * @brief Thin wrapper around fftw memory allocation routines.
 */
class DSPXX_API allocator_base {
public:
	/*!
	 * Call @c fftw_malloc() to allocate memory.
	 * @param size number of bytes to allocate.
	 * @return pointer to memory chunk aligned for SIMD/Altivec ops.
	 * @throw std::bad_alloc on failure.
	 */
	static void* allocate(size_t size);
	/*!
	 * Call @c fftw_free() to deallocate object.
	 * @param p pointer to deallocate.
	 */
	static void deallocate(void* p);
};

/*!
 * @brief STL-conformant allocator based on @c fftw_malloc()/@c fftw_free(),
 * for ensuring right alignment for SIMD/Altivec instructions.
 * @copydoc std::allocator
 */
template<typename T>
class allocator: private allocator_base {
public :
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

	inline allocator() {}
	inline ~allocator() {}
	inline allocator(allocator const&) {}
	template<typename U>
	inline allocator(allocator<U> const&) {}

	inline pointer address(reference r) { return &r; }
	inline const_pointer address(const_reference r) { return &r; }

	inline pointer allocate(size_type cnt, const T* p = 0)
	{return reinterpret_cast<pointer>(allocator_base::allocate(cnt * sizeof(T)));}

	inline void deallocate(pointer p, size_type)
	{allocator_base::deallocate(p);}

	inline size_type max_size() const
	{return std::numeric_limits<size_type>::max() / sizeof(T);}

	inline void construct(pointer p, const T& t) {new (p) T(t);}
	inline void destroy(pointer p) {p->~T();}

	inline bool operator==(allocator const&) { return true; }
	inline bool operator!=(allocator const& a) { return !operator==(a); }
};

} } }

#endif // !DSP_FFTW_DISABLED

#endif /* DSP_FFTW_ALLOCATOR_H_INCLUDED */
