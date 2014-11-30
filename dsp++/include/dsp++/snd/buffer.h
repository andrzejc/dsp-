/*!
 * @file dsp++/snd/buffer.h
 * @brief Buffer layout definitions
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_BUFFER_H_INCLUDED
#define DSP_SND_BUFFER_H_INCLUDED

#include <dsp++/export.h>

namespace dsp { namespace snd {

//! @brief Describes layout of samples/channels in a memory buffer.
struct buffer_layout {
	buffer_layout(unsigned ss, unsigned cs): sample_stride(ss), channel_stride(cs) {}

	unsigned sample_stride;			//!< Byte offset between first bytes of consecutive samples belonging to single channel.
	unsigned channel_stride;		//!< Byte offset between first bytes of samples belonging to consecutive channels, forming a single frame (sampled at the same time instant).

	//! @param[in] channel index of channel
	//! @param[in] frame index of frame
	//! @return offset of first byte of <code>index</code>th sample belonging to specified frame.
	unsigned offset_of(unsigned channel, unsigned frame) {
		return channel * channel_stride + frame * sample_stride;
	}

	//! @return buffer_layout for planar sample buffer with specified configuration
	static buffer_layout planar(unsigned channel_count, unsigned length, unsigned sample_bytes) {
		return buffer_layout(sample_bytes, sample_bytes * length);
	}

	//! @return buffer_layout for interleaved sample buffer with specified configuration
	static buffer_layout interleaved(unsigned channel_count, unsigned length, unsigned sample_bytes) {
		return buffer_layout(sample_bytes * channel_count, sample_bytes);
	}
};

namespace simple_buffer_layout { enum selector {
	planar,
	interleaved
}; } // namespace simple_buffer_layout

struct buffer_info: public buffer_layout {
	unsigned channel_count;		//!< Number of channels stored in buffer
	unsigned length;			//!< Buffer length in frames (number of samples in each channel)
	
	//! @brief Construct buffer_info object using simple_buffer_layout selector for buffer_layout.
	//! @param[in] cc channel count
	//! @param[in] len buffer length in frames
	//! @param[in] sb sample container size in bytes
	//! @param[in] sel simple_buffer_layout selector
	buffer_info(unsigned cc, unsigned len, unsigned sb, simple_buffer_layout::selector sel = simple_buffer_layout::interleaved)
	 :	buffer_layout(simple_buffer_layout::interleaved == sel ? interleaved(cc, len, sb) : planar(cc, len, sb))
	 ,	channel_count(cc), length(len)
	{}

	//! @brief Construct buffer_info object using buffer_layout parameters
	//! @param[in] cc channel count
	//! @param[in] len buffer length in frames
	//! @param[in] ss sample stride 
	//! @param[in] cs channel stride
	buffer_info(unsigned cc, unsigned len, unsigned ss, unsigned cs)
	 :	buffer_layout(ss, cs)
	 ,	channel_count(cc), length(len)
	{}

	//! @brief Construct buffer_info object using buffer_layout object
	//! @param[in] cc channel count
	//! @param[in] len buffer length in frames
	//! @param[in] bl buffer layout
	buffer_info(unsigned cc, unsigned len, const buffer_layout& bl)
	 :	buffer_layout(bl)
	 ,	channel_count(cc), length(len)
	{}

};

template<class Sample>
void buffer_deinterleave(const Sample* input, Sample* output, const unsigned channel_count, const unsigned frame_count) {
	for (unsigned c = 0; c < channel_count; ++c) {
		const Sample* in = input + c;
		for (unsigned i = 0; i < frame_count; ++i, in += channel_count, ++output) 
			*output = *in;
	}
}

template<class Sample>
void buffer_interleave(const Sample* input, Sample* output, const unsigned channel_count, const unsigned frame_count) {
	for (unsigned i = 0; i < frame_count; ++i) {
		const Sample* in = input + i;
		for (unsigned c = 0; c < channel_count; ++c, in += frame_count, ++output) 
			*output = *in;
	}
}

template<class Sample>
void mixdown_interleaved(const Sample* input, Sample* output, const unsigned channel_count, const unsigned frame_count) {
	for (unsigned i = 0; i < frame_count; ++i, ++output) {
		*output = *input;
		++input;
		for (unsigned c = 1; c < channel_count; ++c, ++input)
			*output += *input;
		*output /= channel_count;
	}
}

template<class InputIterator, class OutputIterator> 
void mixdown_interleaved(InputIterator begin, InputIterator end, OutputIterator dest, const unsigned channel_count) 
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

}}

#endif /* DSP_SND_BUFFER_H_INCLUDED */
