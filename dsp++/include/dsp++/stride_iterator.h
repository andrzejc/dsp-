/*!
 * @file dsp++/stride_iterator.h
 * @brief Iterator which advances by a specified number of items.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_STRIDE_ITERATOR_H_INCLUDED
#define DSP_STRIDE_ITERATOR_H_INCLUDED

#include <dsp++/config.h>
#include <iterator>
#include <cassert>

namespace dsp {

template<class Iter>
class stride_iterator
{
public:
	typedef typename std::iterator_traits<Iter>::value_type value_type;
	typedef typename std::iterator_traits<Iter>::reference reference;
	typedef typename std::iterator_traits<Iter>::pointer pointer;
	typedef typename std::iterator_traits<Iter>::difference_type difference_type;
	typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;

	stride_iterator(Iter i, difference_type stride): iter_(i), stride_(stride) 
	{ }

	Iter base() const { return iter_;}

	difference_type stride() const { return stride_; }

	reference operator*() const { return *iter_; }
	pointer operator->() const { return &*iter_; }

	bool operator==(const stride_iterator& x) const { return iter_ == x.iter_; }
	bool operator!=(const stride_iterator& x) const { return iter_ != x.iter_; }

	bool operator<(const stride_iterator& x) const { return iter_ < x.iter_; }
	bool operator>(const stride_iterator& x) const { return iter_ > x.iter_; }
	bool operator<=(const stride_iterator& x) const { return iter_ <= x.iter_; }
	bool operator>=(const stride_iterator& x) const { return iter_ >= x.iter_; }

	stride_iterator& operator++() { using std::advance; advance(iter_, stride_); return *this;}

	stride_iterator operator++(int) 
	{
		stride_iterator tmp(*this);
		operator++();
		return tmp;
	}

	stride_iterator& operator--() {using std::advance; advance(iter_, -stride_); return *this;}

	stride_iterator operator--(int) 
	{
		stride_iterator tmp(*this);
		operator--();
		return tmp;
	}

	stride_iterator& operator+=(difference_type n) {using std::advance; advance(iter_, stride_ * n); return *this;}
	stride_iterator& operator-=(difference_type n) {using std::advance; advance(iter_, -stride_ * n); return *this;}

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
	Iter iter_;
	difference_type stride_;
};

template<typename Iter>
inline stride_iterator<Iter> make_stride(Iter it, typename stride_iterator<Iter>::difference_type stride)
{
	return stride_iterator<Iter>(it, stride);
}

template<typename Iter>
inline stride_iterator<Iter> make_stride(Iter it, typename stride_iterator<Iter>::difference_type stride, typename stride_iterator<Iter>::difference_type offset)
{
	using std::advance;
	advance(it, offset);
	return stride_iterator<Iter>(it, stride);
}

template<typename Iter>
inline stride_iterator<Iter> make_stride(Iter it, typename stride_iterator<Iter>::difference_type stride, typename stride_iterator<Iter>::difference_type offset, typename stride_iterator<Iter>::difference_type index)
{
	using std::advance;
	advance(it, offset + index * stride);
	return stride_iterator<Iter>(it, stride);
}

}

#endif /* DSP_STRIDE_ITERATOR_H_INCLUDED */
