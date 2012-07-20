/*!
 * @file pow2_test.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include "pow2_test.h"

#include <dsp++/pow2.h>
#include <stdexcept>

using namespace dsp::test;

void pow2_test::test_ispow2()
{
	CPPUNIT_ASSERT(!dsp::ispow2(0));
	CPPUNIT_ASSERT(dsp::ispow2(1U));
	CPPUNIT_ASSERT(dsp::ispow2(1L));
	CPPUNIT_ASSERT(!dsp::ispow2(-1));
	CPPUNIT_ASSERT(!dsp::ispow2(-2));
	CPPUNIT_ASSERT(dsp::ispow2(2147483648U));
	CPPUNIT_ASSERT(dsp::ispow2(8589934592LL));
	CPPUNIT_ASSERT(dsp::ispow2(8589934592ULL));
}

void pow2_test::test_nextpow2()
{
	CPPUNIT_ASSERT(dsp::nextpow2(0) == 1);
	CPPUNIT_ASSERT(dsp::nextpow2(-1) == 1);
	CPPUNIT_ASSERT(dsp::nextpow2(1) == 1);
	CPPUNIT_ASSERT(dsp::nextpow2(2) == 2);
	CPPUNIT_ASSERT(dsp::nextpow2(3) == 4);
	CPPUNIT_ASSERT(dsp::nextpow2(4294967296) == 4294967296);
	CPPUNIT_ASSERT(dsp::nextpow2(4294967297) == 8589934592);
}

