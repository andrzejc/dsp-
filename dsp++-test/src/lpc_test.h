/*!
 * @file lpc_test.h
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef LPC_TEST_H_
#define LPC_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class lpc_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(lpc_test);
	CPPUNIT_TEST(test_lpc_real);
	CPPUNIT_TEST(test_lpc_complex);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_lpc_real();
	void test_lpc_complex();
};

} /* namespace test */
} /* namespace dsp */
#endif /* LPC_TEST_H_ */
