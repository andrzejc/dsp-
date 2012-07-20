/*!
 * @file pow2_test.h
 * @brief TestFixture for unit testing dsp++/fft.h artifacts.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef POW2_TEST_H_
#define POW2_TEST_H_

#include <dsp++/pow2.h>
#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

	class pow2_test: public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(pow2_test);
		CPPUNIT_TEST(test_ispow2);
		CPPUNIT_TEST(test_nextpow2);
		CPPUNIT_TEST_SUITE_END();

	public:
		void test_ispow2();
		void test_nextpow2();
	};

} } /* namespace dsp::test */

#endif /* POW2_TEST_H_ */
