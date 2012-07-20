/*!
 * @file dsp++/snd/reader.h
 * @brief Declaration of class dsp::snd::reader.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_SND_READER_H_INCLUDED
#define DSP_SND_READER_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_SNDFILE_DISABLED

#include <dsp++/snd/iobase.h>

namespace dsp { namespace snd {

	//! @brief Simple interface for reading sound files (uses libsndfile as a back-end).
	class DSPXX_API reader: public dsp::snd::iobase
	{
	public:
		/*!
		 * @brief Constructor.
		 * To open the actual stream use one of the open() methods.
		 */
		reader();
		/*!
		 * @name Frame-based input.
		 */
		///@{
		/*!
		 * @brief Read up to count frames from sound file into buf.
		 * @param buf buffer with with enough space to hold count * channel_count() samples.
		 * @param count number of frames to read.
		 * @return number of frames read (0 if EOF is reached).
		 * @throw sndfile_error is thrown if error is reported by underlying libsndfile.
		 */
		size_type read_frames(float* buf, size_type count);
		size_type read_frames(short* buf, size_type count);
		size_type read_frames(int* buf, size_type count);
		size_type read_frames(double* buf, size_type count);
		///@}

		/*!
		 * @name Sample-based input.
		 */
		///@{
		/*!
		 * @brief Read up to count samples from sound file into buf.
		 * @param buf buffer with with enough space to hold count samples.
		 * @param count number of samples to read (must be integer multiply of channel_count()).
		 * @return number of samples read (0 if EOF is reached).
		 * @throw sndfile_error is thrown if error is reported by underlying libsndfile.
		 */
		size_type read_samples(float* buf, size_type count);
		size_type read_samples(short* buf, size_type count);
		size_type read_samples(int* buf, size_type count);
		size_type read_samples(double* buf, size_type count);
		///@}
	};

} }

#endif // !DSP_SNDFILE_DISABLED

#endif /* DSP_SND_READER_H_INCLUDED */
