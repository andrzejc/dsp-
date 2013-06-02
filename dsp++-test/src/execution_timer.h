/*
 * execution_timer.h
 *
 *  Created on: 31-05-2013
 *      Author: Andrzej
 */

#ifndef EXECUTION_TIMER_H_
#define EXECUTION_TIMER_H_

#ifdef __posix__
# include <sys/time.h>
#endif

#include <string>

namespace dsp { namespace test {

class execution_timer {
public:

	execution_timer();

	void start(const char* run);
	void stop();
	void next(const char* run);

private:
	std::string run_;
#ifdef _WIN32
	unsigned long ft_;
	unsigned long pc_;
	unsigned long tp_;
#endif

#ifdef __posix__
	timeval tv_;
#endif

};

} /* namespace test */ } /* namespace dsp */

#endif /* EXECUTION_TIMER_H_ */
