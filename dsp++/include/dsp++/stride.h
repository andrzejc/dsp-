/*!
 * @file dsp++/stride.h
 * @brief Iterator which advances by a specified number of items.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_STRIDE_ITERATOR_H_INCLUDED
#define DSP_STRIDE_ITERATOR_H_INCLUDED

#include <iterator>
#include <cassert>

#include <dsp++/concept_checks.h>

namespace dsp {

template<class InputIterator>
class stride_iterator
{
	DSP_CONCEPT_ASSERT((boost::InputIterator<InputIterator>));
public:

	typedef typename std::iterator_traits<InputIterator>::value_type value_type;
	typedef typename std::iterator_traits<InputIterator>::reference reference;
	typedef typename std::iterator_traits<InputIterator>::pointer pointer;
	typedef typename std::iterator_traits<InputIterator>::difference_type difference_type;
	typedef typename std::iterator_traits<InputIterator>::iterator_category iterator_category;

	stride_iterator(InputIterator i, difference_type stride): iter_(i), stride_(stride)
	{ }

	InputIterator base() const { return iter_;}

	difference_type stride() const { return stride_; }

	reference operator*() const { return *iter_; }
	pointer operator->() const { return &*iter_; }

	bool operator==(const stride_iterator& x) const { return iter_ == x.iter_; }
	bool operator!=(const stride_iterator& x) const { return iter_ != x.iter_; }

	bool operator<(const stride_iterator& x) const { return iter_ < x.iter_; }
	bool operator>(const stride_iterator& x) const { return iter_ > x.iter_; }
	bool operator<=(const stride_iterator& x) const { return iter_ <= x.iter_; }
	bool operator>=(const stride_iterator& x) const { return iter_ >= x.iter_; }

	stride_iterator& operator++() { std::advance(iter_, stride_); return *this;}

	stride_iterator operator++(int)
	{
		stride_iterator tmp(*this);
		++(*this);
		return tmp;
	}

	stride_iterator& operator--() {std::advance(iter_, -stride_); return *this;}

	stride_iterator operator--(int)
	{
		stride_iterator tmp(*this);
		--(*this);
		return tmp;
	}

	stride_iterator& operator+=(difference_type n) {std::advance(iter_, stride_ * n); return *this;}
	stride_iterator& operator-=(difference_type n) {std::advance(iter_, -stride_ * n); return *this;}

	friend stride_iterator operator+(stride_iterator i, difference_type n)
	{
		return i += n;
	}

	friend stride_iterator operator+(difference_type n, stride_iterator i)
	{
		return i += n;
	}

	friend stride_iterator operator-(stride_iterator i, difference_type n)
	{
		return i -= n;
	}

	friend difference_type operator-(stride_iterator i, stride_iterator j)
	{
		assert((i.stride_ == j.stride_));
		return (i.iter_ - j.iter_) / i.stride_;
	}

private:
	InputIterator iter_;
	difference_type stride_;
};

template<typename InputIterator>
inline stride_iterator<InputIterator> make_stride(InputIterator it,
                                                  typename stride_iterator<InputIterator>::difference_type stride)
{
	return stride_iterator<InputIterator>(it, stride);
}

template<typename InputIterator>
inline stride_iterator<InputIterator> make_stride(InputIterator it,
                                                  typename stride_iterator<InputIterator>::difference_type stride,
                                                  typename stride_iterator<InputIterator>::difference_type offset)
{
	std::advance(it, offset);
	return stride_iterator<InputIterator>(it, stride);
}

template<typename InputIterator>
inline stride_iterator<InputIterator> make_stride(InputIterator it,
                                                  typename stride_iterator<InputIterator>::difference_type stride,
                                                  typename stride_iterator<InputIterator>::difference_type offset,
                                                  typename stride_iterator<InputIterator>::difference_type index)
{
	std::advance(it, offset + index * stride);
	return stride_iterator<InputIterator>(it, stride);
}

}

#endif /* DSP_STRIDE_ITERATOR_H_INCLUDED */
