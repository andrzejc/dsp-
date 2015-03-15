/*!
 * @file dsp++/buffer_traits.h
 * @brief Utility template for specifying buffer alignment, padding and allocation method.
 */
#ifndef DSP_BUFFER_TRAITS_H_INCLUDED
#define DSP_BUFFER_TRAITS_H_INCLUDED
#pragma once

#include <memory>	// for std::allocator
#include <cstddef> 	// for offsetof

namespace dsp {

#if __cplusplus < 201103L
namespace detail {template<class T> struct alignof_helper {char c; T member;};}
# define DSP_ALIGNOF(type) offsetof(dsp::detail::alignof_helper<type>, member)
#else
# define DSP_ALIGNOF(type) alignof(type)
#endif

template<class Elem>
struct buffer_traits
{
	typedef Elem value_type;
	typedef std::allocator<Elem> allocator_type;

	static size_t alignment() {return DSP_ALIGNOF(Elem);}
	static size_t padding_size(size_t count) {return 0;}
	static size_t aligned_count(size_t count) {return count;}

};

}

#endif /* DSP_BUFFER_TRAITS_H_INCLUDED */
