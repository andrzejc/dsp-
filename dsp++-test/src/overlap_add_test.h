/*!
 * @file overlap_add_test.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef OVERLAP_ADD_TEST_H_
#define OVERLAP_ADD_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class overlap_add_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(overlap_add_test);
	CPPUNIT_TEST(test_ola);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_ola();
};

} /* namespace test */
} /* namespace dsp */
#endif /* OVERLAP_ADD_TEST_H_ */
