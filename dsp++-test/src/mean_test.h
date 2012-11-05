/*!
 * @file mean_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef MEAN_TEST_H_
#define MEAN_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class mean_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(mean_test);
	CPPUNIT_TEST(test_arithmetic);
	CPPUNIT_TEST(test_quadratic);
	CPPUNIT_TEST(test_geometric);
	CPPUNIT_TEST(test_harmonic);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_arithmetic();
	void test_quadratic();
	void test_geometric();
	void test_harmonic();
};

} /* namespace test */
} /* namespace dsp */
#endif /* MEAN_TEST_H_ */
