#ifndef DSP_IOPORT_H_INCLUDED
#define DSP_IOPORT_H_INCLUDED
#pragma once
#include <iterator>

namespace dsp {

template<class ConstIter>
class ioport_ro {
public:
	typedef ConstIter const_iterator;
	typedef typename std::iterator_traits<const_iterator>::value_type value_type;
	typedef std::pair<const_iterator, const_iterator> const_range_type;

	const_iterator begin() const {return begin_;}
	const_iterator end() const {return end_;}
	const_range_type range() const {return const_range_type(begin_, end_);}

	ioport_ro(const_iterator begin, const_iterator end): begin_(begin), end_(end) {}
	ioport_ro(const_iterator begin, size_t N): begin_(begin), end_(do_advance(begin, N)) {}

protected:
	template<class Iter>
	static Iter do_advance(Iter it, size_t N) {std::advance(it, N); return it;}
private:
	const const_iterator begin_, end_;
};

template<class Value>
class ioport_ro<const Value*> {
public:
	typedef const Value* const_iterator;
	typedef typename std::iterator_traits<const_iterator>::value_type value_type;
	typedef std::pair<const_iterator, const_iterator> const_range_type;

	const_iterator begin() const {return begin_;}
	const_iterator end() const {return end_;}
	const_range_type range() const {return const_range_type(begin_, end_);}

	ioport_ro(const_iterator begin, const_iterator end): begin_(begin), end_(end) {}
	ioport_ro(const_iterator begin, size_t N): begin_(begin), end_(begin + N) {}

protected:
	const Value* begin_;
	const Value* end_;
};

template<class ConstIter, class Iter>
class ioport_rw: public ioport_ro<ConstIter> {
	typedef ioport_ro<ConstIter> base;
public:
	typedef Iter iterator;
	typedef typename std::iterator_traits<iterator>::value_type value_type;
	typedef std::pair<iterator, iterator> range_type;

	iterator begin() {return begin_;}
	iterator end() {return end_;}
	range_type range() {return range_type(begin_, end_);}

	ioport_rw(const_iterator ro_begin, const_iterator ro_end, iterator rw_begin, iterator rw_end): base(ro_begin, ro_end), begin_(rw_begin), end_(rw_end) {}
	ioport_rw(const_iterator ro_begin, iterator rw_begin, size_t N): base(ro_begin, N), begin_(rw_begin), end_(base::do_advance(rw_begin, N)) {}

private:
	const iterator begin_, end_;
};

template<class Value>
class ioport_rw<const Value*, Value*>: public ioport_ro<const Value*>
{
	typedef ioport_ro<const Value*> base;
public:
	typedef Value* iterator;
	typedef typename std::iterator_traits<iterator>::value_type value_type;
	typedef std::pair<iterator, iterator> range_type;

	iterator begin() {return const_cast<Value*>(base::begin_);}
	iterator end() {return const_cast<Value*>(base::end_);}
	range_type range() {return range_type(const_cast<Value*>(base::begin_), const_cast<Value*>(base::end_));}

	ioport_rw(const Value* ro_begin, const Value* ro_end, iterator, iterator): base(ro_begin, ro_end) {}
	ioport_rw(const Value* ro_begin, iterator, size_t N): base(ro_begin, N) {}

	ioport_rw(const Value* ro_begin, const Value* ro_end): base(ro_begin, ro_end) {}
	ioport_rw(const Value* ro_begin, size_t N): base(ro_begin, N) {}

};

}

#endif // DSP_IOPORT_H_INCLUDED