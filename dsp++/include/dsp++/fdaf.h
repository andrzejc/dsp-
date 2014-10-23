/*!
 * @file dsp++/fdaf.h
 * @brief Frequency-Domain Adaptive Filters.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_FDAF_H_INCLUDED
#define DSP_FDAF_H_INCLUDED
#pragma once

#include <dsp++/config.h>
#include <dsp++/fft.h>
#include <dsp++/pow2.h>
#include <dsp++/algorithm.h>
#include <dsp++/noncopyable.h>

#include <algorithm>
#include <functional>

#if !DSP_BOOST_CONCEPT_CHECKS_DISABLED
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>
#endif // !DSP_BOOST_CONCEPT_CHECKS_DISABLED

namespace dsp {

template<class Real, template<class, class> class DFT = dsp::fft>
class fdaf_fast_lms: private noncopyable
{
public:
	typedef Real value_type;
	typedef std::complex<value_type> complex_type;
	typedef DFT<value_type, complex_type> transform_type;
	typedef DFT<complex_type, value_type> inverse_transform_type;
	typedef typename transform_type::input_allocator real_allocator;
	typedef typename transform_type::output_allocator complex_allocator;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;
	typedef complex_type* complex_iterator;
	typedef const complex_type* const_complex_iterator;

private:
	const size_t N_;		//!< frame & impulse response length
	value_type* const x_;	//!< (2N) input/output vector, x[0..N) is old (saved) input frame frame, x[N..2N) is the new input frame upon input or output frame upon return

};

}

#endif /* DSP_FDAF_H_INCLUDED */
