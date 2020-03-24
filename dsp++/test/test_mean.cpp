/*!
 * @file mean_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <dsp++/mean.h>

#include <gtest/gtest.h>

TEST(mean, arithmetic)
{
	float y[150];
	dsp::arithmetic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	EXPECT_TRUE(y[149] = 75.5f);
}

TEST(mean, quadratic)
{
	float y[150];
	dsp::quadratic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	EXPECT_TRUE(y[149] = 87.035433397362169f);
}

TEST(mean, geometric)
{
	float y[150];
	dsp::geometric_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	EXPECT_TRUE(y[149] = 56.456327368458709f);
}

TEST(mean, harmonic)
{
	float y[150];
	dsp::harmonic_mean<float> am(150);
	for (int i = 0; i < 150; ++i)
		y[i] = am(i + 1.f);
	EXPECT_TRUE(y[149] = 26.827965511373677f);
}
