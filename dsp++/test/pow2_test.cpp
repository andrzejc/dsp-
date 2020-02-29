/*!
 * @file pow2_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <dsp++/pow2.h>
#include <stdexcept>

#include <gtest/gtest.h>

TEST(pow2, ispow2)
{
	EXPECT_TRUE(!dsp::ispow2(0));
	EXPECT_TRUE(dsp::ispow2(1U));
	EXPECT_TRUE(dsp::ispow2(1L));
	EXPECT_TRUE(!dsp::ispow2(-1));
	EXPECT_TRUE(!dsp::ispow2(-2));
	EXPECT_TRUE(dsp::ispow2(2147483648U));
	EXPECT_TRUE(dsp::ispow2(8589934592LL));
	EXPECT_TRUE(dsp::ispow2(8589934592ULL));
}

TEST(pow2, nextpow2)
{
	EXPECT_TRUE(dsp::nextpow2(0) == 1);
	EXPECT_TRUE(dsp::nextpow2(-1) == 1);
	EXPECT_TRUE(dsp::nextpow2(1) == 1);
	EXPECT_TRUE(dsp::nextpow2(2) == 2);
	EXPECT_TRUE(dsp::nextpow2(3) == 4);
	EXPECT_TRUE(dsp::nextpow2(4294967296) == 4294967296);
	EXPECT_TRUE(dsp::nextpow2(4294967297) == 8589934592);
}
