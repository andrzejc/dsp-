/*!
 * @file dsp++/doc.h
 * @brief Documentation of common and generic artifacts, like namespaces, forward declarations etc.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_DOC_H_INCLUDED
#define DSP_DOC_H_INCLUDED

/*!
 * @brief Algorithms and utilities for Digital Signal Processing for C++.
 */
namespace dsp {

	/*!
	 * @brief Algorithms and tools strictly specific to sound (audio) processing.
	 * These include support for reading/writing sound files, DAFX-algorithms, etc.
	 */
	namespace snd {}

	/*!
	 * @brief Window function generators.
	 * @see http://en.wikipedia.org/wiki/Window_function
	 */
	namespace wnd {}

	/*!
	 * @brief DFT (and related) operators based on libfftw3.
	 */
	namespace fftw {}
}

#endif /* DSP_DOC_H_INCLUDED */
