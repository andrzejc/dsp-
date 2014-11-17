/*!
 * @file adaptfilt_test.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef ADAPTFILT_TEST_H_
#define ADAPTFILT_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class adaptfilt_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(adaptfilt_test);
	CPPUNIT_TEST(test_lms);
	CPPUNIT_TEST(test_nlms);
	CPPUNIT_TEST(test_fdaf_overlap_save);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_lms();
	void test_nlms();

	void test_fdaf_overlap_save();
};

} /* namespace test */
} /* namespace dsp */
#endif /* ADAPTFILT_TEST_H_ */
