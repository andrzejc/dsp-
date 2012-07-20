/*!
 * @file dsp++/buffer.h
 * @brief Class buffer for continuous buffering (partitioning signal into frames).
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_BUFFER_H_INCLUDED
#define DSP_BUFFER_H_INCLUDED

#include <utility>

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
template<class Elem, class Storage = boost::circular_buffer<Elem> > class buffer: public Storage
{
public:

	typedef Storage base_type;
	typedef typename base_type::value_type value_type;
	typedef typename base_type::pointer pointer;
	typedef typename base_type::const_pointer const_pointer;
	typedef typename base_type::reference reference;
	typedef typename base_type::const_reference const_reference;
	typedef typename base_type::difference_type difference_type;
	typedef typename base_type::size_type size_type;
	typedef typename base_type::allocator_type allocator_type;
	typedef typename base_type::const_iterator const_iterator;
	typedef typename base_type::iterator iterator;
	typedef typename base_type::const_reverse_iterator const_reverse_iterator;
	typedef typename base_type::reverse_iterator reverse_iterator;
	typedef typename base_type::capacity_type capacity_type;
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
	 :	base_type(capacity)
	 ,	frame_(frame_size)
	 ,	overlap_(overlap)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("frame_length must not be greater than buffer capacity");
		if (overlap > 0)
		{
			if (size_type(overlap) >= frame_size)
				throw std::invalid_argument("overlap must be less than frame_length");
			insert(base_type::begin(), overlap, initial_value);
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
	explicit buffer(capacity_type capacity, size_type frame_size,
			size_type overlap, const_pointer initial_values)
	 :	base_type(capacity)
	 ,	frame_(frame_size)
	 ,	overlap_(overlap)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("dsp::buffer frame_length must not be greater than buffer capacity");
		if (overlap >= frame_size)
			throw std::invalid_argument("dsp::buffer overlap must be less than frame_length");
		insert(base_type::begin(), initial_values, initial_values + overlap);
	}

	explicit buffer(capacity_type capacity, size_type frame_size,
			size_type overlap, const nodelay_t tag)
	 : 	base_type(capacity)
	 ,	frame_(frame_size)
	 ,	overlap_(overlap)
	{
		if (frame_size > capacity)
			throw std::invalid_argument("dsp::buffer frame_length must not be greater than buffer capacity");
		if (overlap >= frame_size)
			throw std::invalid_argument("dsp::buffer overlap must be less than frame_length");
	}

	size_type frame_count() const
	{return base_type::size() / size_type(difference_type(frame_) - overlap_);}

	size_type frame_size() const {return frame_;}
	difference_type frame_overlap() const {return overlap_;}

	iterator frame_begin(size_type n = 0)
	{return base_type::begin() + n * size_type(difference_type(frame_) - overlap_);}
	const_iterator frame_begin(size_type n = 0) const
	{return base_type::begin() + n * size_type(difference_type(frame_) - overlap_);}

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
		base_type::rerase(base_type::begin(), base_type::begin() + num);
	}

	void pop_frame()
	{pop_frames(1);}

	//! @return true, if the number of stored elements is not less than configured frame size.
	bool has_frame() const {return base_type::size() >= frame_;}


private:
	size_type frame_;
	difference_type overlap_;
};

}

#endif /* DSP_BUFFER_H_INCLUDED */
