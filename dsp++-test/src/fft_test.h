/*!
 * @file fft_test.h
 * @brief TestFixture for unit testing dsp++/fft.h artifacts.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#ifndef FFT_TEST_H_
#define FFT_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

	class fft_test: public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(fft_test);
		CPPUNIT_TEST(test_fft_64);
		CPPUNIT_TEST(test_fft_65);
		CPPUNIT_TEST(test_fft_536870912);
		CPPUNIT_TEST(test_fft_equals_fftw);
		CPPUNIT_TEST(test_fft_r2c);
		CPPUNIT_TEST(test_fft_two_way);
		CPPUNIT_TEST_SUITE_END();

	public:
		void test_fft_64();
		void test_fft_65();
		void test_fft_536870912();
		void test_fft_equals_fftw();
		void test_fft_r2c();
		void test_fft_two_way();
	};

} } /* namespace dsp::test */

#endif /* FFT_TEST_H_ */
