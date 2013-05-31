/*!
 * @file mean_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef SIMD_TEST_H_
#define SIMD_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class simd_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(simd_test);
	CPPUNIT_TEST(test_mulv);
	CPPUNIT_TEST(test_muls);
	CPPUNIT_TEST(test_mulc);
	CPPUNIT_TEST(test_divv);
	CPPUNIT_TEST(test_divs);
	CPPUNIT_TEST(test_addv);
	CPPUNIT_TEST(test_adds);
	CPPUNIT_TEST(test_subv);
	CPPUNIT_TEST(test_subs);
	CPPUNIT_TEST(test_dot);
	CPPUNIT_TEST(test_dotc);
	CPPUNIT_TEST(test_sqrt);
	CPPUNIT_TEST(test_recip);
	CPPUNIT_TEST(test_rsqrt);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_mulv();
	void test_muls();
	void test_mulc();
	void test_divv();
	void test_divs();
	void test_addv();
	void test_adds();
	void test_subv();
	void test_subs();
	void test_dot();
	void test_dotc();
	void test_sqrt();
	void test_recip();
	void test_rsqrt();
};

} /* namespace test */
} /* namespace dsp */
#endif /* SIMD_TEST_H_ */
