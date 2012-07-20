/*!
 * @file xcorr_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef XCORR_TEST_H_
#define XCORR_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class xcorr_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(xcorr_test);
	CPPUNIT_TEST(test_acorr_none);
	CPPUNIT_TEST(test_acorr_biased);
	CPPUNIT_TEST(test_acorr_unbiased);
	CPPUNIT_TEST(test_acorr_coeff);
	CPPUNIT_TEST(test_xcorr_none);
	CPPUNIT_TEST(test_xcorr_biased);
	CPPUNIT_TEST(test_xcorr_unbiased);
	CPPUNIT_TEST(test_xcorr_coeff);
	CPPUNIT_TEST(test_acorr_complex_none);
//	CPPUNIT_TEST(test_acorr_complex_biased);
//	CPPUNIT_TEST(test_acorr_complex_unbiased);
//	CPPUNIT_TEST(test_acorr_complex_coeff);
	CPPUNIT_TEST(test_xcorr_complex_none);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_acorr_none();
	void test_acorr_biased();
	void test_acorr_unbiased();
	void test_acorr_coeff();
	void test_xcorr_none();
	void test_xcorr_biased();
	void test_xcorr_unbiased();
	void test_xcorr_coeff();
	void test_acorr_complex_none();
//	void test_acorr_complex_biased();
//	void test_acorr_complex_unbiased();
//	void test_acorr_complex_coeff();
	void test_xcorr_complex_none();
};

} /* namespace test */
} /* namespace dsp */
#endif /* XCORR_TEST_H_ */
