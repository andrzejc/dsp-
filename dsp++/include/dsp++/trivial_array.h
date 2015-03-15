/*!
 * @file dsp++/trivial_array.h
 * @brief Implementation of a simple, noncopyable array template capable of using custom memory allocator.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_TRIVIAL_ARRAY_H_INCLUDED
#define DSP_TRIVIAL_ARRAY_H_INCLUDED

#include <dsp++/export.h>
#include <dsp++/noncopyable.h>

#include <memory>
#include <cstddef>


namespace dsp {

struct uninitialized_t {};
extern const uninitialized_t uninitialized;

template<class Elem, class Alloc = std::allocator<Elem> >
class trivial_array: private noncopyable
{
	typedef typename Alloc::template rebind<Elem>::other this_allocator;

	void destroy()
	{
		this_allocator a(alloc_);
		if (init_) {
			pointer p = arr_;
			for (size_t i = 0; i < size_; ++i, ++p)
				a.destroy(p);
			init_ = false;
		}
		a.deallocate(arr_, size_);
	}

public:
	typedef Alloc 								allocator_type;
	typedef Elem 								value_type;
    typedef typename this_allocator::pointer           	pointer;
    typedef typename this_allocator::const_pointer     	const_pointer;
    typedef typename this_allocator::reference         	reference;
    typedef typename this_allocator::const_reference   	const_reference;
    typedef pointer 							iterator;
    typedef const_pointer 						const_iterator;
    typedef size_t					 			size_type;
    typedef ptrdiff_t					 		difference_type;

    pointer get() {return arr_;}
    const_pointer get() const {return arr_;}

    iterator begin() {return arr_;}
    const_iterator begin() const {return arr_;}
    iterator end() {return arr_ + size_;}
    const_iterator end() const {return arr_ + size_;}

    reference operator[](size_type n) {return arr_[n];}
    const_reference operator[](size_type n) const {return arr_[n];}

    explicit trivial_array(size_t size, const value_type& val = value_type(), const allocator_type& a = allocator_type())
     :	alloc_(a)
     ,	arr_(this_allocator(alloc_).allocate(size))
     ,	size_(size)
     ,	init_(false)
    {std::uninitialized_fill_n(arr_, size, val); init_ = true;}

    explicit trivial_array(size_t size, const uninitialized_t&, const allocator_type& a = allocator_type())
     :	alloc_(a)
     ,	arr_(this_allocator(alloc_).allocate(size))
     ,	size_(size)
     ,	init_(false)
    {}

    ~trivial_array() {destroy();}

    size_type size() const {return size_;}
    size_type length() const {return size_;}

private:
    allocator_type alloc_;
	pointer	arr_;
	size_type size_;
	bool init_;
};

}

#endif /* DSP_TRIVIAL_ARRAY_H_INCLUDED */
