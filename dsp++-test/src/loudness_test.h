/*!
 * @file loudness_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef LOUDNESS_TEST_H_
#define LOUDNESS_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class loudness_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(loudness_test);
	CPPUNIT_TEST(test_ebu1);
	CPPUNIT_TEST(test_peak);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_ebu1();
	void test_peak();
};

} /* namespace test */
} /* namespace dsp */
#endif /* LOUDNESS_TEST_H_ */
