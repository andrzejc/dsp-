/*!
 * @file intmath_test.cpp
 *
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */
#include "intmath_test.h"
#include <climits>

#include <dsp++/intmath.h>

using namespace dsp::test;

void intmath_test::test_signum()
{
	CPPUNIT_ASSERT(dsp::signum(-10) == -1);
	CPPUNIT_ASSERT(dsp::signum(INT_MIN) == -1);
	CPPUNIT_ASSERT(dsp::signum(0) == 0);
	CPPUNIT_ASSERT(dsp::signum(100) == 1);
	CPPUNIT_ASSERT(dsp::signum(INT_MAX) == 1);
	CPPUNIT_ASSERT(dsp::signum(0u) == 0u);
	CPPUNIT_ASSERT(dsp::signum(123u) == 1u);
}

void intmath_test::test_add()
{
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(1, 1) == 2);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(10, -10) == 0);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(-100, -100) == -200);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(-1000, 1000) == 0);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(INT_MAX, 1000) < 0);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(INT_MIN, -1000) > 0);

	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(10000u, 1u) == 10001u);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::fastest>(0u, 0u) == 0u);

	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(1, 1) == 2);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(1, INT_MAX) == INT_MAX);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(-1, INT_MIN) == INT_MIN);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(INT_MAX - 100, 110) == INT_MAX);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(INT_MIN + 1000, -1001) == INT_MIN);
	CPPUNIT_ASSERT(dsp::add<dsp::overflow::saturate>(1u, UINT_MAX) == UINT_MAX);

	CPPUNIT_ASSERT_NO_THROW(dsp::add<dsp::overflow::exception>(INT_MAX - 10, 10));
	CPPUNIT_ASSERT_NO_THROW(dsp::add<dsp::overflow::exception>(INT_MIN + 100, 100));
	CPPUNIT_ASSERT_THROW(dsp::add<dsp::overflow::exception>(INT_MAX - 10, 11), std::overflow_error);
	CPPUNIT_ASSERT_THROW(dsp::add<dsp::overflow::exception>(INT_MIN + 11, -15), std::overflow_error);
}

void intmath_test::test_sub()
{
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(1, 1) == 0);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(10, -10) == 20);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(-100, -100) == 0);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(-1000, 1000) == -2000);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(INT_MIN, 1000) > 0);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(INT_MAX, -1000) < 0);

	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(10000u, 1u) == 9999u);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(0u, 0u) == 0u);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::fastest>(0u, 100u) > 100u);

	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::saturate>(1, 1) == 0);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::saturate>(INT_MIN, 100) == INT_MIN);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::saturate>(INT_MAX, -110) == INT_MAX);
	CPPUNIT_ASSERT(dsp::sub<dsp::overflow::saturate>(1u, UINT_MAX) == 0);

	CPPUNIT_ASSERT_NO_THROW(dsp::sub<dsp::overflow::exception>(INT_MIN + 10, 10));
	CPPUNIT_ASSERT_NO_THROW(dsp::sub<dsp::overflow::exception>(INT_MAX - 100, -100));
	CPPUNIT_ASSERT_THROW(dsp::sub<dsp::overflow::exception>(INT_MAX - 10, -11), std::overflow_error);
	CPPUNIT_ASSERT_THROW(dsp::sub<dsp::overflow::exception>(INT_MIN + 11, 15), std::overflow_error);
}

void intmath_test::test_mul()
{
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(1, 1) == 1);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(10, -10) == -100);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(-100, -100) == 10000);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(-1000, 1000) == -1000000);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(INT_MAX, 2) < 0);

	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(10000u, 1u) == 10000u);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(0u, 0u) == 0u);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::fastest>(0u, 100u) == 0u);

	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(1, 1) == 1);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(INT_MIN, 100) == INT_MIN);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(INT_MIN, -100) == INT_MAX);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(INT_MAX, 110) == INT_MAX);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(INT_MAX, -110) == INT_MIN);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(INT_MIN, -1) == INT_MAX);
	CPPUNIT_ASSERT(dsp::mul<dsp::overflow::saturate>(2u, UINT_MAX) == UINT_MAX);

	CPPUNIT_ASSERT_NO_THROW(dsp::mul<dsp::overflow::exception>(INT_MIN, 1));
	CPPUNIT_ASSERT_NO_THROW(dsp::mul<dsp::overflow::exception>(INT_MAX, -1));
	CPPUNIT_ASSERT_THROW(dsp::mul<dsp::overflow::exception>(INT_MIN, -1), std::overflow_error);
	CPPUNIT_ASSERT_THROW(dsp::mul<dsp::overflow::exception>(INT_MAX, 2), std::overflow_error);
}

void intmath_test::test_round()
{
	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(19234, 0) == 19234));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(19234, 0) == 19234));
	CPPUNIT_ASSERT((dsp::round<rounding::negative>(19234, -1) == 19234));
	CPPUNIT_ASSERT((dsp::round<rounding::positive>(19234, -15) == 19234));

	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(10, 3) == 8));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(10, 3) == 8));
	CPPUNIT_ASSERT((dsp::round<rounding::negative>(10, 3) == 8));
	CPPUNIT_ASSERT((dsp::round<rounding::positive>(10, 3) == 16));

	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(12, 3) == 8));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(12, 3) == 16));
	CPPUNIT_ASSERT((dsp::round<rounding::negative>(12, 3) == 8));
	CPPUNIT_ASSERT((dsp::round<rounding::positive>(12, 3) == 16));

	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(-10, 3) == -8));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(-10, 3) == -8));
	CPPUNIT_ASSERT((dsp::round<rounding::negative>(-10, 3) == -16));
	CPPUNIT_ASSERT((dsp::round<rounding::positive>(-10, 3) == -8));

	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(-12, 3) == -8));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(-12, 3) == -16));
	CPPUNIT_ASSERT((dsp::round<rounding::negative>(-12, 3) == -16));
	CPPUNIT_ASSERT((dsp::round<rounding::positive>(-12, 3) == -8));

	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(short(32767), 15) == 0));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(short(32767), 15) == 0));
	CPPUNIT_ASSERT((dsp::round<rounding::truncated>(short(-32768), 15) == 0));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest>(short(-32768), 15) == 0));

	CPPUNIT_ASSERT_NO_THROW((dsp::round<rounding::truncated, overflow::exception>(short(32767), 14)));
	CPPUNIT_ASSERT((dsp::round<rounding::truncated, overflow::exception>(short(32767), 14) == 16384));
	CPPUNIT_ASSERT_THROW((dsp::round<rounding::nearest, overflow::exception>(short(32767), 14)), std::overflow_error);
	CPPUNIT_ASSERT_NO_THROW((dsp::round<rounding::nearest, overflow::exception>(short(-32767), 14)));
	CPPUNIT_ASSERT((dsp::round<rounding::nearest, overflow::exception>(short(-32767), 14) == short(-32768)));
}

void intmath_test::test_check_overflow()
{
	short val = 16383;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14));
	++val;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14), std::overflow_error);
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	val = 32767;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 16));
	val = -16384;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14));
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	--val;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 14), std::overflow_error);
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 15));
	val = 0;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(val, 0));
	++val;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(val, 0), std::overflow_error);

	unsigned short uval = 16383;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 14));
	++uval;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 14), std::overflow_error);
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15));
	uval = 32767;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15));
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 16));
	uval = 32768;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 15), std::overflow_error);
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 16));
	uval = 0;
	CPPUNIT_ASSERT_NO_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 0));
	++uval;
	CPPUNIT_ASSERT_THROW(dsp::overflow_check_handle<overflow::exception>(uval, 0), std::overflow_error);
}
