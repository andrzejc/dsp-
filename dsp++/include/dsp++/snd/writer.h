/*!
 * @file dsp++/snd/writer.h
 * @brief Declaration of class dsp::snd::writer.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef DSP_SND_WRITER_H_INCLUDED
#define DSP_SND_WRITER_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_SNDFILE_DISABLED

#include <dsp++/snd/iobase.h>

namespace dsp { namespace snd {

//! @brief Simple interface for writing sound files (uses libsndfile as a back-end).
class DSPXX_API writer: public dsp::snd::iobase
{
public:
	/*!
	 * @brief Constructor.
	 * To open the actual stream use one of the open() methods.
	 */
	writer();
	/*!
	 * @name Frame-based output.
	 */
	///@{
	/*!
	 * @brief Write count frames from buf to the sound file.
	 * @param buf buffer holding count * channel_count() samples.
	 * @param count number of frames to write.
	 * @return number of frames written.
	 * @ingroup Frame
	 * @throw sndfile_error is thrown if error is reported by underlying libsndfile.
	 */
	size_type write_frames(const float* buf, size_type count);
	size_type write_frames(const short* buf, size_type count);
	size_type write_frames(const int* buf, size_type count);
	size_type write_frames(const double* buf, size_type count);
	///@}

	/*!
	 * @name Sample-based output.
	 */
	///@{
	/*!
	 * @brief Write count samples from buf to the sound file.
	 * @param buf buffer holding count samples.
	 * @param count number of samples to write (must be integer multiply of channel_count()).
	 * @return number of samples written.
	 * @ingroup Sample
	 * @throw sndfile_error is thrown if error is reported by underlying libsndfile.
	 */
	size_type write_samples(const float* buf, size_type count);
	size_type write_samples(const short* buf, size_type count);
	size_type write_samples(const int* buf, size_type count);
	size_type write_samples(const double* buf, size_type count);
	///@}
};

} }

#endif // !DSP_SNDFILE_DISABLED

#endif /* DSP_SND_WRITER_H_INCLUDED */
