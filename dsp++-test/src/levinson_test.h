/*!
 * @file levinson_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef LEVINSON_TEST_H_
#define LEVINSON_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class levinson_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(levinson_test);
	CPPUNIT_TEST(test_levinson);
	CPPUNIT_TEST(test_levinson_complex);
	CPPUNIT_TEST(test_levinson_short);
	CPPUNIT_TEST(test_levdown);
	CPPUNIT_TEST(test_levup);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_levinson();
	void test_levinson_complex();
	void test_levinson_short();
	void test_levdown();
	void test_levup();
};

} /* namespace test */
} /* namespace dsp */
#endif /* LEVINSON_TEST_H_ */
