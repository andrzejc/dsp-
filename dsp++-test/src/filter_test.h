/*!
 * @file filter_test.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef FILTER_TEST_H_
#define FILTER_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class filter_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(filter_test);
	CPPUNIT_TEST(test_fir);
	CPPUNIT_TEST(test_fir_block);
	CPPUNIT_TEST(test_iir);
	CPPUNIT_TEST(test_iir_block);
	CPPUNIT_TEST(test_sos);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_fir();
	void test_fir_block();
	void test_iir();
	void test_iir_block();
	void test_sos();
};

} /* namespace test */
} /* namespace dsp */
#endif /* FILTER_TEST_H_ */
