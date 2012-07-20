/*!
 * @file lattice_test.h
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef LATTICE_TEST_H_
#define LATTICE_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

namespace dsp { namespace test {

/*
 *
 */
class lattice_test: public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(lattice_test);
	CPPUNIT_TEST(test_fir);
	CPPUNIT_TEST(test_iir);
	CPPUNIT_TEST(test_lattice_ladder);
	CPPUNIT_TEST_SUITE_END();
public:
	void test_fir();
	void test_iir();
	void test_lattice_ladder();
};

} /* namespace test */
} /* namespace dsp */
#endif /* LATTICE_TEST_H_ */
