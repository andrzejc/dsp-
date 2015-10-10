/*!
 * @file mean_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <boost/test/auto_unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp> 

#include <dsp++/mean.h>

BOOST_AUTO_TEST_SUITE(mean)

BOOST_AUTO_TEST_CASE(arithmetic)
{
	float y[150];
	dsp::arithmetic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 75.5f);
}

BOOST_AUTO_TEST_CASE(quadratic)
{
	float y[150];
	dsp::quadratic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 87.035433397362169f);
}

BOOST_AUTO_TEST_CASE(geometric)
{
	float y[150];
	dsp::geometric_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 56.456327368458709f);
}

BOOST_AUTO_TEST_CASE(harmonic)
{
	float y[150];
	dsp::harmonic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 26.827965511373677f);
}

BOOST_AUTO_TEST_SUITE_END()
