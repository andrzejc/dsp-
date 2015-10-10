/*
 * execution_timer.cpp
 *
 *  Created on: 31-05-2013
 *      Author: Andrzej
 */

#include "execution_timer.h"

#ifdef _WIN32
# include <windows.h>
#endif

#include <cstring>
#include <climits>
#include <cstdio>

dsp::test::execution_timer::execution_timer()
#ifdef _WIN32
 : ft_(0), pc_(::GetPriorityClass(::GetCurrentProcess())), tp_(::GetThreadPriority(::GetCurrentThread()))
#endif
{
#if defined(__posix__) || defined(__MACH__)
	std::memset(&tv_, 0, sizeof(tv_));
#endif
}

void dsp::test::execution_timer::start(const char* run) {
	run_ = run;
#ifdef _WIN32
	::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	ft_ = ::GetTickCount();
#endif

#if defined(__posix__) || defined(__MACH__)
	gettimeofday(&tv_, NULL);
#endif
}

void dsp::test::execution_timer::stop() {
	unsigned long millis = 0;
#ifdef _WIN32
	unsigned long now = ::GetTickCount();
	if (now >= ft_)
		millis = now - ft_;
	else
		millis = (ULONG_MAX - ft_ + now + 1);
	::SetPriorityClass(::GetCurrentProcess(), pc_);
	::SetThreadPriority(::GetCurrentThread(), tp_);
#endif

#if defined(__posix__) || defined(__MACH__)
	timeval ntv;
	gettimeofday(&ntv, NULL);
	millis = 1000 * (ntv.tv_sec - tv_.tv_sec);
	millis += (ntv.tv_usec - tv_.tv_usec + 500) / 1000;
#endif

	printf("\n%s time: %lu ms\n", run_.c_str(), millis);
}

void dsp::test::execution_timer::next(const char* run) {
	stop();
	start(run);
}


