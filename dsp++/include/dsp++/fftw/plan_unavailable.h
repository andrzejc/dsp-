/*!
 * @file plan_unavailable.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef PLAN_UNAVAILABLE_H_
#define PLAN_UNAVAILABLE_H_

#include <stdexcept>
#include <dsp++/export.h>

namespace dsp { namespace fftw {

	class DSPXX_API plan_unavailable: public std::runtime_error {
	public:
		plan_unavailable(): runtime_error("dsp::fftw::plan_unavailable") {}
		~plan_unavailable() throw();
	};
}}

#endif /* PLAN_UNAVAILABLE_H_ */
