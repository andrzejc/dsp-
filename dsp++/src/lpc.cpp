/*!
 * @file lpc.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#include <dsp++/lpc.h>

#include <stdexcept>

size_t dsp::lpc_base::verify_length(const size_t L)
{
	if (0 == L)
		throw std::domain_error("dsp::lpc input length must be positive");
	return L;
}

size_t dsp::lpc_base::verify_order(const size_t P)
{
	if (0 == P)
		return L_ - 1;
	if (P >= L_)
		throw std::domain_error("dsp::lpc prediction order must not be greater than input length");
	return P;
}

size_t dsp::lpc_base::verify_dft_length(const size_t dft_size, const size_t idft_size)
{
	if (dft_size != idft_size)
		throw std::logic_error("dsp::lpc forward/inverse DFT transform size mismatch");
	if (dft_size < 2 * L_ - 1)
		throw std::domain_error("dsp:lpc DFT transform length too short for given input frame size");
	return dft_size;
}

