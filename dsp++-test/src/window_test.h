/*!
 * @file window_test.h
 * @brief TestFixture for unit testing dsp++/window.h artifacts.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef WINDOW_TEST_H_
#define WINDOW_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

	class window_test: public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(window_test);
		CPPUNIT_TEST(test_rectwin);
		CPPUNIT_TEST(test_hamming);
		CPPUNIT_TEST(test_hann);
		CPPUNIT_TEST(test_blackman);
		CPPUNIT_TEST(test_kaiser);
		CPPUNIT_TEST(test_gausswin);
		CPPUNIT_TEST_SUITE_END();

	public:
		void test_rectwin();
		void test_hamming();
		void test_hann();
		void test_blackman();
		void test_kaiser();
		void test_gausswin();
	};

} } /* namespace dsp::test */

#endif /* WINDOW_TEST_H_ */
