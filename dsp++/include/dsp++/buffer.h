/*!
 * @file dsp++/buffer.h
 * @brief Class buffer for continuous buffering (partitioning signal into frames).
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_BUFFER_H_INCLUDED
#define DSP_BUFFER_H_INCLUDED

#include <utility>

// TODO make dependecny on boost::circular_buffer configurable with DSP_BOOST_DISABLED
#include <boost/circular_buffer.hpp>

namespace dsp {

//! @brief Tagging struct to choose appropriate variant of dsp::buffer constructor.
//! @internal implementation detail.
struct nodelay_t {};
//! @brief Constant passed to dsp::buffer constructor to choose "nodelay" variant.
//! This is more expressive than using bool.
extern const nodelay_t nodelay;

/*!
 *	@brief Circular-buffer based version of MATLAB's buffer function, for
 *	online partitioning of signal data as it arrives.
 * 	@see http://www.mathworks.com/help/toolbox/signal/ref/buffer.html for exhaustive
 * 	explanation of the buffering mechanics.
 * 	@tparam Elem type of stored elements
 * 	@tparam Storage collection used as a storage.
 */
template<class Elem, class Storage = boost::circular_buffer<Elem> > class buffer
{
public:

	typedef Storage storage_type;
	typedef typename storage_type::value_type value_type;
	typedef typename storage_type::pointer pointer;
	typedef typename storage_type::const_pointer const_pointer;
	typedef typename storage_type::reference reference;
	typedef typename storage_type::const_reference const_reference;
	typedef typename storage_type::difference_type difference_type;
	typedef typename storage_type::size_type size_type;
	typedef typename storage_type::allocator_type allocator_type;
	typedef typename storage_type::const_iterator const_iterator;
	typedef typename storage_type::iterator iterator;
	typedef typename storage_type::const_reverse_iterator const_reverse_iterator;
	typedef typename storage_type::reverse_iterator reverse_iterator;
	typedef typename storage_type::capacity_type capacity_type;
	typedef std::pair<const_iterator, const_iterator> const_iterator_range;
	typedef std::pair<iterator, iterator> iterator_range;

	/*!
	 * @param capacity maximum buffer capacity
	 * @param frame_size size of the output frame partition.
	 * @param overlap
	 * @param initial_value
	 * @throw std::invalid_argument if frame_length is greater than buffer capacity,
	 * overlap is positive and not less than frame_length or negative and the sum of
	 * frame_size and (-overlap) is greater than buffer capacity.
	 */
	explicit buffer(capacity_type capacity, size_type frame_size,
			difference_type overlap = 0, const_reference initial_value = value_type())
	 :	frame_(frame_size)
	 ,	overlap_(overlap)
	 ,	buf_(capacity)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("frame_length must not be greater than buffer capacity");
		if (overlap > 0)
		{
			if (size_type(overlap) >= frame_size)
				throw std::invalid_argument("overlap must be less than frame_length");
			buf_.insert(buf_.begin(), overlap, initial_value);
		}
		else if (overlap < 0)
		{
			if (size_type(difference_type(frame_size) - overlap) > capacity)
				throw std::invalid_argument("frame_length with underlap exceeds capacity");
		}
	}

	/*!
	 * @param capacity maximum buffer capacity.
	 * @param frame_size size of the output frame partition.
	 * @param overlap number of samples from the last frame to repeat at the beginning of the
	 * following frame (this constructor version requires non-negative overlap).
	 * @param initial_values the buffer is pre-filled with overlap frames taken from
	 * initial_values vector
	 */
	explicit buffer(capacity_type capacity, size_type frame_size, size_type overlap, const_pointer initial_values)
	 :	frame_(frame_size)
	 ,	overlap_(overlap)
	 ,	buf_(capacity)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("dsp::buffer frame_length must not be greater than buffer capacity");
		if (overlap >= frame_size)
			throw std::invalid_argument("dsp::buffer overlap must be less than frame_length");
		buf_.insert(buf_.begin(), initial_values, initial_values + overlap);
	}

	explicit buffer(capacity_type capacity, size_type frame_size, size_type overlap, const nodelay_t tag)
	 :	frame_(frame_size)
	 ,	overlap_(overlap)
	 ,	buf_(capacity)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("dsp::buffer frame_length must not be greater than buffer capacity");
		if (overlap >= frame_size)
			throw std::invalid_argument("dsp::buffer overlap must be less than frame_length");
	}

	size_type size() const {return buf_.size();}

	size_type frame_count() const
	{
		size_type sz = buf_.size();
		if (sz < frame_)
			return 0;
		else
			return 1 + (sz - frame_) / size_type(difference_type(frame_) - overlap_);
	}

	size_type frame_size() const {return frame_;}
	difference_type frame_overlap() const {return overlap_;}

	iterator frame_begin(size_type n = 0)
	{return buf_.begin() + n * size_type(difference_type(frame_) - overlap_);}
	const_iterator frame_begin(size_type n = 0) const
	{return buf_.begin() + n * size_type(difference_type(frame_) - overlap_);}

	iterator frame_end(size_type n  = 0) {return frame_begin(n) + frame_;}
	const_iterator frame_end(size_type n = 0) const {return frame_begin(n) + frame_;}

	iterator_range frame(size_type n = 0)
	{return std::make_pair(frame_begin(n), frame_end(n));}

	const_iterator_range frame(size_type n = 0) const
	{return std::make_pair(frame_begin(n), frame_end(n));}

	//! Remove count first frames from the buffer.
	//! @param count number of frames to remove.
	void pop_frames(size_type count)
	{
		size_type num = count * size_type(difference_type(frame_) - overlap_);
		buf_.rerase(buf_.begin(), buf_.begin() + num);
	}

	void pop_frame()
	{pop_frames(1);}

	//! @return true, if the number of stored elements is not less than configured frame size.
	bool has_frame() const {return buf_.size() >= frame_;}

	iterator begin() {return buf_.begin();}
	const_iterator begin() const {return buf_.begin();}
	iterator end() {return buf_.end();}
	const_iterator end() const {return buf_.end();}

	iterator insert(iterator pos, const value_type& val) {return buf_.insert(pos, val);}
	template<class InputIterator>
	void insert(iterator pos, InputIterator first, InputIterator last) {buf_.insert(pos, first, last);}

	void push_back(const value_type& val) {buf_.push_back(val);}

private:
	size_type frame_;
	difference_type overlap_;
	storage_type buf_;
};

// TODO this is work in progress and needs to be reworked (as well as dsp::buffer which must not use boost::circular_buffer)
template<class Elem, class Storage = std::vector<Elem> > 
class overlap {
public:
	typedef Storage storage_type;
	typedef typename storage_type::iterator iterator;
	typedef typename storage_type::const_iterator const_iterator;
	typedef typename storage_type::value_type value_type;
	typedef typename storage_type::reference reference;
	typedef typename storage_type::const_reference const_reference;


	// TODO check if overlap < frame_length
	overlap(size_t frame_length, size_t overlap)
	 :	frame_(frame_length)
	 ,	overlap_(overlap) 
	{
		if (overlap >= frame_length)
			throw std::invalid_argument("frame_length must be greater than overlap");
	}

	size_t size() const {return buf_.size();}
	size_t free_size() const {
		size_t sz = size();
		return (sz < overlap_ ? 0 : (sz - overlap_));
	}

	size_t frame_count() const {return size() / frame_;}
	size_t free_frame_count() const {return free_size() / frame_;}

	size_t frame_length() const {return frame_;}
	size_t overlap_length() const {return overlap_;}

	iterator begin() {return buf_.begin();}
	iterator end() {return buf_.end();}
	const_iterator begin() const {return buf_.begin();}
	const_iterator end() const {return buf_.end();}

	iterator frame_begin(size_t n = 0) {return buf_.begin() + n * frame_;}
	const_iterator frame_begin(size_t n = 0) const {return buf_.begin() + n * frame_;}
	iterator frame_end(size_t n = 0) {return buf_.begin() + (n + 1) * frame_;}
	const_iterator frame_end(size_t n = 0) const {return buf_.begin() + (n + 1) * frame_;}

	void pop_frames(size_t count) {
		size_t num = count * frame_;
		assert(num <= size());
		buf_.erase(buf_.begin(), buf_.begin() + num);
	}

	void pop_frame() {pop_frames(1);}

	template<class InputIterator>
	void push_frame(InputIterator it) {
		if (buf_.empty()) {
			buf_.resize(frame_);
			std::copy_n(it, frame_, buf_.begin());
		}
		else {
			assert(buf_.size() >= overlap_);
			buf_.resize(buf_.size() + (frame_ - overlap_));
			iterator dest = buf_.end() - frame_;
			for (unsigned i = 0; i < overlap_; ++i, ++dest, ++it) {
				double mix = double(i + 1) / (overlap_ + 1);
				*dest = static_cast<Elem>((1. - mix) * (*dest) + mix * (*it));
			}
			std::copy_n(it, (frame_ - overlap_), dest);
		}
	}

private:
	size_t frame_, overlap_;
	storage_type buf_;
};

}

#endif /* DSP_BUFFER_H_INCLUDED */
