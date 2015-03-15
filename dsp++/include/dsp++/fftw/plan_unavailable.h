/*!
 * @file plan_unavailable.h
 * Declaration of dsp::dft::fftw::plan_unavailable exception.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef PLAN_UNAVAILABLE_H_
#define PLAN_UNAVAILABLE_H_

#include <stdexcept>
#include <dsp++/export.h>

namespace dsp { namespace dft { namespace fftw {

	/// @brief Thrown when FFTW3 planner wrapper is unable to design a plan for given parameters.
	class DSPXX_API plan_unavailable: public std::runtime_error {
	public:
		plan_unavailable(): runtime_error("dsp::dft::fftw::plan_unavailable") {}
		~plan_unavailable() throw();
	};
}}}

#endif /* PLAN_UNAVAILABLE_H_ */
