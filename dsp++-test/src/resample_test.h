/*!
 * @file resample_test.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef RESAMPLE_TEST_H_
#define RESAMPLE_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class resample_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(resample_test);
	CPPUNIT_TEST(test_interpolator);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_interpolator();
};

} /* namespace test */
} /* namespace dsp */
#endif /* RESAMPLE_TEST_H_ */
