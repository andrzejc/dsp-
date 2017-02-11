/*!
 * @file dsp++/snd/buffer.h
 * @brief Buffer layout definitions
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_BUFFER_H_INCLUDED
#define DSP_SND_BUFFER_H_INCLUDED

#include <dsp++/export.h>
#include <dsp++/config.h>
#include <dsp++/stride.h>
#include <dsp++/algorithm.h>
#include <dsp++/concept_checks.h>

namespace dsp { namespace snd { namespace buf {

//! @brief Describes layout of samples/channels in a memory buffer.
struct layout
{
	//! @param ss sample stride
	//! @param cs channel stride
	layout(unsigned ss, unsigned cs): sample_stride(ss), channel_stride(cs) {}

	unsigned sample_stride;			//!< Byte offset between first bytes of consecutive samples belonging to single
									//!< channel.
	unsigned channel_stride;		//!< Byte offset between first bytes of samples belonging to consecutive channels,
									//!< forming a single frame (sampled at the same time instant).

	//! @param[in] frame index of frame
	//! @param[in] channel index of channel
	//! @return offset of first byte of `index`th sample belonging to specified frame.
	unsigned offset_of(unsigned frame, unsigned channel) const {
		return channel * channel_stride + frame * sample_stride;
	}

	//! @return layout for planar sample buffer with specified configuration. Planar buffer is the one in which
	//!         samples belonging to one channel form continuous sequences.
	static layout planar(unsigned length, unsigned channel_count, unsigned sample_bytes) {
		return layout(sample_bytes, sample_bytes * length);
	}

	//! @return layout for interleaved sample buffer with specified configuration. Interleaved buffer is the one
	//!         in which samples belonging to a single frame form continuous sequences.
	static layout interleaved(unsigned length, unsigned channel_count, unsigned sample_bytes) {
		return layout(sample_bytes * channel_count, sample_bytes);
	}
};

//! @brief Extends @p layout memory-spatial information with information about number of channels
//! and length in frames.
struct info: public layout
{
	unsigned channel_count;		//!< Number of channels stored in buffer
	unsigned length;			//!< Buffer length in frames (number of samples in each channel)

	//! @brief Type of @p layout "constructor" function.
	//! @see layout::interleaved()
	typedef layout (*selector)(unsigned length, unsigned channel_count, unsigned sample_bytes);

	//! @brief Construct info object using simple_layout selector for layout.
	//! @param[in] len buffer length in frames
	//! @param[in] cc channel count
	//! @param[in] sb sample container size in bytes
	//! @param[in] sel selector or @p layout "constructor" function
	info(unsigned len, unsigned cc, unsigned sb, selector sel = layout::interleaved)
	 :	layout(sel(cc, len, sb))
	 ,	channel_count(cc), length(len)
	{}

	//! @brief Construct info object using layout parameters
	//! @param[in] len buffer length in frames
	//! @param[in] cc channel count
	//! @param[in] ss sample stride
	//! @param[in] cs channel stride
	info(unsigned len, unsigned cc, unsigned ss, unsigned cs)
	 :	layout(ss, cs)
	 ,	channel_count(cc), length(len)
	{}

	//! @brief Construct info object using layout object
	//! @param[in] len buffer length in frames
	//! @param[in] cc channel count
	//! @param[in] bl buffer layout
	info(unsigned len, unsigned cc, const layout& bl)
	 :	layout(bl)
	 ,	channel_count(cc), length(len)
	{}

};

// TODO rewrite functions below using stride_iterator for consistency

//! @brief Copy samples from interleaved buffer @p input into planar buffer @p output, assuming @p channel_count
//! channels in the pack and @p frame_count of frames.
//!
//! @tparam InputIterator with value type convertible to @p OutputIterator's value type.
//! @tparam OutputIterator
//!
//! @param[in] input iterator used for reading interleaved sample data.
//! @param[out] output iterator to which output planar data will be written.
//! @param[in] channel_count number of samples in a single frame.
//! @param[in] frame_count number of frames to deinterleave.
template<class InputIterator, class OutputIterator>
DSP_CONCEPT_REQUIRES(
	((boost::InputIterator<InputIterator>))
	((boost::OutputIterator<OutputIterator, typename std::iterator_traits<InputIterator>::value_type>)),
(void))
deinterleave(InputIterator input,
             const unsigned frame_count,
             OutputIterator output,
             const unsigned channel_count)
{
	for (unsigned c = 0; c < channel_count; ++c) {
		output = dsp::copy_n(dsp::make_stride(input, channel_count, c),
		                     frame_count,
		                     output);
	}
}

template<class InputIterator, class OutputIterator>
DSP_CONCEPT_REQUIRES(
	((boost::InputIterator<InputIterator>))
	((boost::OutputIterator<OutputIterator, typename std::iterator_traits<InputIterator>::value_type>)),
(void))
interleave(InputIterator input,
           const unsigned frame_count,
           OutputIterator output,
           const unsigned channel_count)
{
	for(unsigned c = 0; c < channel_count; ++c) {
		dsp::copy_n(input,
		            frame_count,
		            dsp::make_stride(output, channel_count, c));
	}
}

template<class ISample, class OSample>
DSP_CONCEPT_REQUIRES(
	((boost::Convertible<ISample, OSample>)),
(void))
interleave(const ISample* input, const unsigned frame_count, OSample* output, const unsigned channel_count) {
	for (unsigned i = 0; i < frame_count; ++i) {
		const ISample* in = input + i;
		for (unsigned c = 0; c < channel_count; ++c, in += frame_count, ++output)
			*output = *in;
	}
}

template<class ISample, class OSample>
DSP_CONCEPT_REQUIRES(
	((boost::Convertible<ISample, OSample>)),
(void))
mixdown_interleaved(const ISample* input, const unsigned frame_count, OSample* output, const unsigned channel_count)
{
	for (unsigned i = 0; i < frame_count; ++i, ++output) {
		*output = *input;
		++input;
		for (unsigned c = 1; c < channel_count; ++c, ++input)
			*output += *input;
		*output /= channel_count;
	}
}

template<class InputIterator, class OutputIterator>
DSP_CONCEPT_REQUIRES(
	((boost::InputIterator<InputIterator>))
	((boost::OutputIterator<OutputIterator, typename std::iterator_traits<InputIterator>::value_type>)),
(void))
mixdown_interleaved(InputIterator begin, InputIterator end, OutputIterator dest, const unsigned channel_count)
{
	if (1 == channel_count)
		std::copy(begin, end, dest);
	else {
		for (; begin != end; ++dest) {
			*dest = *begin;
			++begin;
			for (unsigned c = 1; c < channel_count; ++c, ++begin)
				*dest += *begin;
			*dest /= channel_count;
		}
	}
}

}}}

#endif /* DSP_SND_BUFFER_H_INCLUDED */
