/*!
 * @file intmath_test.h
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef INTMATH_TEST_H_
#define INTMATH_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class intmath_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(intmath_test);
	CPPUNIT_TEST(test_signum);
	CPPUNIT_TEST(test_add);
	CPPUNIT_TEST(test_sub);
	CPPUNIT_TEST(test_mul);
	CPPUNIT_TEST(test_round);
	CPPUNIT_TEST(test_check_overflow);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_signum();
	void test_add();
	void test_sub();
	void test_mul();
	void test_round();
	void test_check_overflow();
};

} /* namespace test */
} /* namespace dsp */
#endif /* INTMATH_TEST_H_ */
