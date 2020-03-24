/*!
 * @file dsp++/snd/sndfile/writer.h
 * @brief Declaration of class dsp::snd::sndfile::writer.
 */
#pragma once
#include <dsp++/config.h>

#if !DSPXX_LIBSNDFILE_DISABLED

#include <dsp++/snd/io.h>
#include <dsp++/snd/sndfile/iobase.h>

namespace dsp { namespace snd { namespace sndfile {

//! @brief Simple interface for writing sound files (uses libsndfile as a back-end).
// TODO make detail::iobase private
class DSPXX_API writer: public snd::writer, public detail::iobase {
public:
	/*!
	 * @brief Constructor.
	 * To open the actual stream use one of the open() methods.
	 */
	writer():
		iobase{mode::write}
	{}
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
	 * @throw sndfile::error is thrown if error is reported by underlying libsndfile.
	 */
	size_t write_frames(const float* buf, size_t count) override;
	size_t write_frames(const short* buf, size_t count) override;
	size_t write_frames(const int* buf, size_t count) override;
	size_t write_frames(const double* buf, size_t count) override;
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
	 * @throw sndfile::error is thrown if error is reported by underlying libsndfile.
	 */
	size_t write_samples(const float* buf, size_t count);
	size_t write_samples(const short* buf, size_t count);
	size_t write_samples(const int* buf, size_t count);
	size_t write_samples(const double* buf, size_t count);
	///@}

    bool supports_metadata() const override { return iobase::supports_metadata(); }
    void set_string(const char* metadata_str, const char* val, size_t val_length) override { iobase::set_string(metadata_str, val, val_length); }
};

}}}

#endif // !DSPXX_LIBSNDFILE_DISABLED
