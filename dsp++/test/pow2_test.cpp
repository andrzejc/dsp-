/*!
 * @file pow2_test.cpp
 * 
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp> 

#include <dsp++/pow2.h>
#include <stdexcept>

BOOST_AUTO_TEST_SUITE(pow2)

BOOST_AUTO_TEST_CASE(ispow2)
{
	BOOST_CHECK(!dsp::ispow2(0));
	BOOST_CHECK(dsp::ispow2(1U));
	BOOST_CHECK(dsp::ispow2(1L));
	BOOST_CHECK(!dsp::ispow2(-1));
	BOOST_CHECK(!dsp::ispow2(-2));
	BOOST_CHECK(dsp::ispow2(2147483648U));
	BOOST_CHECK(dsp::ispow2(8589934592LL));
	BOOST_CHECK(dsp::ispow2(8589934592ULL));
}

BOOST_AUTO_TEST_CASE(nextpow2)
{
	BOOST_CHECK(dsp::nextpow2(0) == 1);
	BOOST_CHECK(dsp::nextpow2(-1) == 1);
	BOOST_CHECK(dsp::nextpow2(1) == 1);
	BOOST_CHECK(dsp::nextpow2(2) == 2);
	BOOST_CHECK(dsp::nextpow2(3) == 4);
	BOOST_CHECK(dsp::nextpow2(4294967296) == 4294967296);
	BOOST_CHECK(dsp::nextpow2(4294967297) == 8589934592);
}

BOOST_AUTO_TEST_SUITE_END()
