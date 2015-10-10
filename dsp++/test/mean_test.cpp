/*!
 * @file mean_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include "mean_test.h"

#include <dsp++/mean.h>
#include <stdexcept>

using namespace dsp::test;

void mean_test::test_arithmetic()
{
	float y[150];
	dsp::arithmetic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 75.5f);
}

void mean_test::test_quadratic()
{
	float y[150];
	dsp::quadratic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 87.035433397362169f);
}

void mean_test::test_geometric()
{
	float y[150];
	dsp::geometric_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 56.456327368458709f);
}

void mean_test::test_harmonic()
{
	float y[150];
	dsp::harmonic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	BOOST_CHECK(y[149] = 26.827965511373677f);
}
