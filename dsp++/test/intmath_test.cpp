/*!
 * @file intmath_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include <dsp++/intmath.h>

#include <gtest/gtest.h>

#include <climits>
#include <cstdint>

using namespace dsp;

TEST(intmath, test_signum)
{
	EXPECT_TRUE(dsp::signum(-10) == -1);
	EXPECT_TRUE(dsp::signum(INT_MIN) == -1);
	EXPECT_TRUE(dsp::signum(0) == 0);
	EXPECT_TRUE(dsp::signum(100) == 1);
	EXPECT_TRUE(dsp::signum(INT_MAX) == 1);
	EXPECT_TRUE(dsp::signum(0u) == 0u);
	EXPECT_TRUE(dsp::signum(123u) == 1u);
}

TEST(intmath, test_add)
{
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(1, 1) == 2);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(10, -10) == 0);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(-100, -100) == -200);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(-1000, 1000) == 0);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(INT_MAX, 1000) < 0);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(INT_MIN, -1000) > 0);

	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(10000u, 1u) == 10001u);
	EXPECT_TRUE(dsp::add<dsp::overflow::fastest>(0u, 0u) == 0u);

	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(1, 1) == 2);
	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(1, INT_MAX) == INT_MAX);
	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(-1, INT_MIN) == INT_MIN);
	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(INT_MAX - 100, 110) == INT_MAX);
	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(INT_MIN + 1000, -1001) == INT_MIN);
	EXPECT_TRUE(dsp::add<dsp::overflow::saturate>(1u, UINT_MAX) == UINT_MAX);

	EXPECT_NO_THROW(dsp::add<dsp::overflow::exception>(INT_MAX - 10, 10));
	EXPECT_NO_THROW(dsp::add<dsp::overflow::exception>(INT_MIN + 100, 100));
	EXPECT_THROW(dsp::add<dsp::overflow::exception>(INT_MAX - 10, 11), std::overflow_error);
	EXPECT_THROW(dsp::add<dsp::overflow::exception>(INT_MIN + 11, -15), std::overflow_error);
}

TEST(intmath, test_sub)
{
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(1, 1) == 0);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(10, -10) == 20);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(-100, -100) == 0);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(-1000, 1000) == -2000);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(INT_MIN, 1000) > 0);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(INT_MAX, -1000) < 0);

	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(10000u, 1u) == 9999u);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(0u, 0u) == 0u);
	EXPECT_TRUE(dsp::sub<dsp::overflow::fastest>(0u, 100u) > 100u);

	EXPECT_TRUE(dsp::sub<dsp::overflow::saturate>(1, 1) == 0);
	EXPECT_TRUE(dsp::sub<dsp::overflow::saturate>(INT_MIN, 100) == INT_MIN);
	EXPECT_TRUE(dsp::sub<dsp::overflow::saturate>(INT_MAX, -110) == INT_MAX);
	EXPECT_TRUE(dsp::sub<dsp::overflow::saturate>(1u, UINT_MAX) == 0);

	EXPECT_NO_THROW(dsp::sub<dsp::overflow::exception>(INT_MIN + 10, 10));
	EXPECT_NO_THROW(dsp::sub<dsp::overflow::exception>(INT_MAX - 100, -100));
	EXPECT_THROW(dsp::sub<dsp::overflow::exception>(INT_MAX - 10, -11), std::overflow_error);
	EXPECT_THROW(dsp::sub<dsp::overflow::exception>(INT_MIN + 11, 15), std::overflow_error);
}

TEST(intmath, test_mul)
{
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(1, 1) == 1);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(10, -10) == -100);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(-100, -100) == 10000);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(-1000, 1000) == -1000000);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(INT_MAX, 2) < 0);

	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(10000u, 1u) == 10000u);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(0u, 0u) == 0u);
	EXPECT_TRUE(dsp::mul<dsp::overflow::fastest>(0u, 100u) == 0u);

	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(1, 1) == 1);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(INT_MIN, 100) == INT_MIN);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(INT_MIN, -100) == INT_MAX);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(INT_MAX, 110) == INT_MAX);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(INT_MAX, -110) == INT_MIN);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(INT_MIN, -1) == INT_MAX);
	EXPECT_TRUE(dsp::mul<dsp::overflow::saturate>(2u, UINT_MAX) == UINT_MAX);

	EXPECT_NO_THROW(dsp::mul<dsp::overflow::exception>(INT_MIN, 1));
	EXPECT_NO_THROW(dsp::mul<dsp::overflow::exception>(INT_MAX, -1));
	EXPECT_THROW(dsp::mul<dsp::overflow::exception>(INT_MIN, -1), std::overflow_error);
	EXPECT_THROW(dsp::mul<dsp::overflow::exception>(INT_MAX, 2), std::overflow_error);
}

TEST(intmath, test_div)
{
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(1, 1) == 1);
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(10, -10) == -1);
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(-100, -100) == 1);
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(-1000, 1000) == -1);
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(INT_MAX, 2) > 0);

	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(10000u, 2u) == 5000u);
	EXPECT_TRUE(dsp::div<dsp::overflow::fastest>(0u, 100u) == 0u);

	EXPECT_TRUE(dsp::div<dsp::overflow::saturate>(INT_MIN, -1) == INT_MAX);
	EXPECT_NO_THROW(dsp::div<dsp::overflow::exception>(INT_MIN, 1));
	EXPECT_NO_THROW(dsp::div<dsp::overflow::exception>(INT_MAX, -1));
	EXPECT_THROW(dsp::div<dsp::overflow::exception>(INT_MIN, -1), std::overflow_error);
}

TEST(intmath, test_mod)
{
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(1, 1) == 0);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(10, -10) == 0);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(-100, -100) == 0);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(-1000, 1000) == 0);

	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(11, 10) == 1);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(-11, 10) == -1);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(11, -10) == 1);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(-11, -10) == -1);

	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(INT_MAX, 2) == 1);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(INT_MIN, 2) == 0);

	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(10000u, 2u) == 0u);
	EXPECT_TRUE(dsp::mod<dsp::overflow::fastest>(0u, 100u) == 0u);
}

TEST(intmath, test_neg)
{
	EXPECT_TRUE(dsp::neg<dsp::overflow::fastest>(1) == -1);
	EXPECT_TRUE(dsp::neg<dsp::overflow::fastest>(10) == -10);
	EXPECT_TRUE(dsp::neg<dsp::overflow::fastest>(-100) == 100);
	EXPECT_TRUE(dsp::neg<dsp::overflow::fastest>(INT_MAX) == -INT_MAX);

	EXPECT_TRUE(dsp::neg<dsp::overflow::saturate>(10000u) == 0u);
	EXPECT_TRUE(dsp::neg<dsp::overflow::fastest>(0u) == 0u);

	EXPECT_TRUE(dsp::neg<dsp::overflow::saturate>(INT_MIN) == INT_MAX);
	EXPECT_THROW(dsp::neg<dsp::overflow::exception>(INT_MIN), std::overflow_error);
}

TEST(intmath, test_round)
{
	EXPECT_TRUE((dsp::round<rounding::truncated>(19234, 0) == 19234));
	EXPECT_TRUE((dsp::round<rounding::nearest>(19234, 0) == 19234));
	EXPECT_TRUE((dsp::round<rounding::negative>(19234, -1) == 19234));
	EXPECT_TRUE((dsp::round<rounding::positive>(19234, -15) == 19234));

	EXPECT_TRUE((dsp::round<rounding::truncated>(10, 3) == 8));
	EXPECT_TRUE((dsp::round<rounding::nearest>(10, 3) == 8));
	EXPECT_TRUE((dsp::round<rounding::negative>(10, 3) == 8));
	EXPECT_TRUE((dsp::round<rounding::positive>(10, 3) == 16));

	EXPECT_TRUE((dsp::round<rounding::truncated>(12, 3) == 8));
	EXPECT_TRUE((dsp::round<rounding::nearest>(12, 3) == 16));
	EXPECT_TRUE((dsp::round<rounding::negative>(12, 3) == 8));
	EXPECT_TRUE((dsp::round<rounding::positive>(12, 3) == 16));

	EXPECT_TRUE((dsp::round<rounding::truncated>(-10, 3) == -8));
	EXPECT_TRUE((dsp::round<rounding::nearest>(-10, 3) == -8));
	EXPECT_TRUE((dsp::round<rounding::negative>(-10, 3) == -16));
	EXPECT_TRUE((dsp::round<rounding::positive>(-10, 3) == -8));

	EXPECT_TRUE((dsp::round<rounding::truncated>(-12, 3) == -8));
	EXPECT_TRUE((dsp::round<rounding::nearest>(-12, 3) == -16));
	EXPECT_TRUE((dsp::round<rounding::negative>(-12, 3) == -16));
	EXPECT_TRUE((dsp::round<rounding::positive>(-12, 3) == -8));

	EXPECT_TRUE((dsp::round<rounding::truncated>(std::int16_t(32767), 15) == 0));
	EXPECT_TRUE((dsp::round<rounding::nearest>(std::int16_t(32767), 15) == 0));
	EXPECT_TRUE((dsp::round<rounding::truncated>(std::int16_t(-32768), 15) == 0));
	EXPECT_TRUE((dsp::round<rounding::nearest>(std::int16_t(-32768), 15) == 0));

	EXPECT_NO_THROW((dsp::round<rounding::truncated, overflow::exception>(std::int16_t(32767), 14)));
	EXPECT_TRUE((dsp::round<rounding::truncated, overflow::exception>(std::int16_t(32767), 14) == 16384));
	EXPECT_THROW((dsp::round<rounding::nearest, overflow::exception>(std::int16_t(32767), 14)), std::overflow_error);
	EXPECT_NO_THROW((dsp::round<rounding::nearest, overflow::exception>(std::int16_t(-32767), 14)));
	EXPECT_TRUE((dsp::round<rounding::nearest, overflow::exception>(std::int16_t(-32767), 14) == std::int16_t(-32768)));
}

TEST(intmath, test_check_overflow)
{
	std::int16_t val = 16383;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14));
	++val;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14), std::overflow_error);
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	val = 32767;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 16));
	val = -16384;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14));
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	--val;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14), std::overflow_error);
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	val = 0;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 0));
	++val;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 0), std::overflow_error);

	std::uint16_t uval = 16383;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 14));
	++uval;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 14), std::overflow_error);
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15));
	uval = 32767;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15));
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 16));
	uval = 32768;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15), std::overflow_error);
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 16));
	uval = 0;
	EXPECT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 0));
	++uval;
	EXPECT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 0), std::overflow_error);
}

TEST(intmath, test_rint)
{
	EXPECT_TRUE((dsp::rint<std::int16_t>(0.5f, rounding::truncated, overflow::fastest) == 0));
	EXPECT_TRUE((dsp::rint<std::int16_t>(0.9f, rounding::truncated, overflow::fastest) == 0));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-0.9f, rounding::truncated, overflow::fastest) == 0));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-11.f, rounding::truncated, overflow::fastest) == -11));
	EXPECT_TRUE((dsp::rint<std::int16_t>(100.f, rounding::truncated, overflow::fastest) == 100));

	EXPECT_TRUE((dsp::rint<std::int16_t>(0.1, rounding::nearest, overflow::fastest) == 0));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-0.1, rounding::nearest, overflow::fastest) == 0));
	EXPECT_TRUE((dsp::rint<std::int32_t>(100.7, rounding::nearest, overflow::fastest) == 101));
	EXPECT_TRUE((dsp::rint<std::int32_t>(-100.7, rounding::nearest, overflow::fastest) == -101));
	EXPECT_TRUE((dsp::rint<std::int16_t>(32767.6, rounding::positive, overflow::saturate) == 32767));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-32769.6, rounding::positive, overflow::saturate) == -32768));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-32767.6, rounding::negative, overflow::saturate) == -32768));
	EXPECT_TRUE((dsp::rint<std::int16_t>(-32767.6, rounding::positive, overflow::saturate) == -32767));

	EXPECT_THROW((dsp::rint<std::int16_t>(-32768.6, rounding::nearest, overflow::exception)), std::overflow_error);
	EXPECT_THROW((dsp::rint<std::int16_t>(32767.6, rounding::nearest, overflow::exception)), std::overflow_error);
	EXPECT_THROW((dsp::rint<std::uint16_t>(-1., rounding::nearest, overflow::exception)), std::overflow_error);
	EXPECT_THROW((dsp::rint<std::uint16_t>(-1., rounding::nearest, overflow::exception)), std::overflow_error);

	EXPECT_THROW((dsp::rint<int>(-2147483648.5, rounding::nearest, overflow::exception)), std::overflow_error);
	EXPECT_THROW((dsp::rint<int>(2147483647.5, rounding::nearest, overflow::exception)), std::overflow_error);
}

TEST(intmath, test_gcd)
{
	EXPECT_TRUE(dsp::gcd(1, 1) == 1);
	EXPECT_TRUE(dsp::gcd(5, 3) == 1);
	EXPECT_TRUE(dsp::gcd(3, 5) == 1);
	EXPECT_TRUE(dsp::gcd(6, 4) == 2);
	EXPECT_TRUE(dsp::gcd(8, 4) == 4);
	EXPECT_TRUE(dsp::gcd(44100, 48000) == 300);
}
