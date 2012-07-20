/*!
 * @file xcorr.cpp
 */
#include <dsp++/xcorr.h>

#include <stdexcept>

size_t dsp::xcorr_base::verify_input_length(size_t M)
{
	if (0 == M)
		throw std::domain_error("dsp::xcorr input sequence length must be greater than 0");
	return M;
}

size_t dsp::xcorr_base::verify_transform_length(const size_t dft_size, const size_t idft_size)
{
	if (dft_size != idft_size)
		throw std::logic_error("dsp::xcorr forward/inverse DFT transform size mismatch");
	if (dft_size < M_)
		throw std::domain_error("dsp:xcorr DFT transform length too short for given input frame size");
	return dft_size;
}





