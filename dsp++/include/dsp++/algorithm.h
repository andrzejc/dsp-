/*!
 * @file dsp++/algorithm.h
 * @brief Various unassorted algorithms loosely complementing these found in &lt;algorithm&gt;
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_ALGORITHM_H_INCLUDED
#define DSP_ALGORITHM_H_INCLUDED

#include <iterator>
#include <functional>

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>

namespace dsp {

/*!
 * @brief Copy at most n elements from the [src_begin, src_end) sequence into sequence starting at dest.
 * @param src_begin start of sequence to copy.
 * @param src_end end of sequence to copy.
 * @param dest the destination of copy operation.
 * @param n maximum number of elements to copy.
 * @return number of copied elements, min(src_end - src_begin, n).
 * @tparam InputIterator type of the src_begin and src_end iterators, which must conform to
 * input iterator concept.
 * @tparam OutputIterator type of the dest iterator, which must conform to output iterator concept
 * and its value type must be convertible to the value type of InputIterator.
 */
template<class InputIterator, class OutputIterator> inline
BOOST_CONCEPT_REQUIRES(((boost::InputIterator<InputIterator>))
		((boost::OutputIterator<OutputIterator, typename std::iterator_traits<InputIterator>::value_type>)),
		(size_t))
copy_at_most_n(InputIterator src_begin, InputIterator src_end, OutputIterator dest, size_t n)
{
	size_t i;
	for (i = 0; (i < n) && (src_begin != src_end); ++i, ++dest, ++src_begin)
		*dest = *src_begin;
	return i;
}

template<class InputIterator, class OutputIterator> inline
BOOST_CONCEPT_REQUIRES(((boost::InputIterator<InputIterator>))
		((boost::OutputIterator<OutputIterator, typename std::iterator_traits<InputIterator>::value_type>)),
		(OutputIterator))
copy_n(InputIterator src, size_t n, OutputIterator dest)
{
	for (size_t i = 0; i < n; ++i, ++dest, ++src)
		*dest = *src;
	return dest;
}

//template<class Sample> inline
//void unwrap_transform(Sample* vec, size_t length)
//{
//	Sample*
//}

template<class Sample>
struct sample_based_transform: public std::unary_function<Sample, Sample>
{
	typedef Sample sample_type;
};

}

#endif /* DSP_ALGORITHM_H_INCLUDED */
