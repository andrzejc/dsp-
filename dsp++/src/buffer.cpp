/*
 * buffer_test.cpp
 *
 *  Created on: 06-04-2012
 *      Author: andrzej
 */
#include <dsp++/buffer.h>
#include <vector>

using namespace dsp;

const nodelay_t nodelay = {};

#define BUFFER_TESTING
#ifdef BUFFER_TESTING
namespace {
	void test_buffer()
	{
		buffer<float> buf(1024, 256, 128);
		std::vector<float> f1(buf.begin(), buf.frame_end());
	}
}
#endif



