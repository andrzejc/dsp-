/*!
 * @file dsp++/trivial_array.h
 * @brief Implementation of a simple, noncopyable array template capable of using custom memory allocator.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#pragma once
#include <dsp++/types.h>

#include <memory>

namespace dsp {

struct uninitialized_t {};
constexpr uninitialized_t uninitialized = {};

template<class Elem, class Alloc = std::allocator<Elem>>
class trivial_array {
    using this_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<Elem>;

public:
    using allocator_type = Alloc;
    using value_type = Elem;
    using pointer = typename std::allocator_traits<this_allocator>::pointer;
    using const_pointer = typename std::allocator_traits<this_allocator>::const_pointer;
    using reference = Elem&;
    using const_reference = const Elem&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    pointer get() {return arr_;}
    const_pointer get() const {return arr_;}

    iterator begin() {return arr_;}
    const_iterator begin() const {return arr_;}
    iterator end() {return arr_ + size_;}
    const_iterator end() const {return arr_ + size_;}

    reference operator[](size_type n) {return arr_[n];}
    const_reference operator[](size_type n) const {return arr_[n];}

    explicit trivial_array(size_t size, const uninitialized_t&, const allocator_type& a = allocator_type()):
        alloc_(a),
        arr_(std::allocator_traits<this_allocator>::allocate(alloc_, size)),
        size_(size),
        init_(false)
    {}

    explicit trivial_array(size_t size, const value_type& val = value_type(), const allocator_type& a = allocator_type()):
        trivial_array{size, uninitialized, a}
    {
        std::uninitialized_fill_n(arr_, size, val);
        init_ = true;
    }

    trivial_array(const trivial_array&) = delete;
    trivial_array& operator=(const trivial_array&) = delete;

    ~trivial_array() {
        if (init_) {
            pointer p = arr_;
            for (size_t i = 0; i < size_; ++i, ++p) {
                std::allocator_traits<this_allocator>::destroy(alloc_, p);
            }
            init_ = false;
        }
        std::allocator_traits<this_allocator>::deallocate(alloc_, arr_, size_);
    }

    size_type size() const {return size_;}
    size_type length() const {return size_;}

private:
    this_allocator alloc_;
    pointer	arr_;
    size_type size_;
    bool init_;
};

}
