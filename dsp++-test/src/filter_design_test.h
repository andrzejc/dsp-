/*
 * filter_design_test.h
 *
 *  Created on: 08-05-2012
 *      Author: Andrzej
 */

#ifndef FILTER_DESIGN_TEST_H_
#define FILTER_DESIGN_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

	class filter_design_test: public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(filter_design_test);
		CPPUNIT_TEST(test_bp);
		CPPUNIT_TEST(test_hilbert);
		CPPUNIT_TEST(test_differentiator);
		CPPUNIT_TEST(test_butter_bp);
		CPPUNIT_TEST_SUITE_END();

	public:
		void test_bp();
		void test_hilbert();
		void test_differentiator();
		void test_butter_bp();
	};

} } /* namespace dsp::test */

#endif /* FILTER_DESIGN_TEST_H_ */
