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

template<class Elem>
struct buffer_traits
{
	typedef Elem value_type;
	typedef std::allocator<Elem> allocator_type;

	static std::size_t alignment() {return alignof(Elem);}
	static std::size_t padding_size(size_t count) {return 0;}
	static std::size_t aligned_count(size_t count) {return count;}

};

}

#endif /* DSP_BUFFER_TRAITS_H_INCLUDED */
