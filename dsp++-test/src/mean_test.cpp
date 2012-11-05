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
	// TODO implement me!
	dsp::arithmetic_mean<float> am(128);

}

void mean_test::test_quadratic()
{
	// TODO implement me!
	dsp::quadratic_mean<float> am(128);
}

void mean_test::test_geometric()
{
	// TODO implement me!
	dsp::geometric_mean<float> am(128);
}

void mean_test::test_harmonic()
{
	// TODO implement me!
	dsp::harmonic_mean<float> am(128);
}
