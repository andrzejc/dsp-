#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>


using namespace dsp::fi;

typedef fixed<16, 0> Q15_t;

Q15_t m = std::numeric_limits<Q15_t>::min();

BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

fixed<8, 3> fff(-0.33333f);

Q15_t v(0.33333);
Q15_t v1(16, raw);
bool ff = (v == v1);


static bool test1() {
	fixed<32, 12> res = multiply_lossless<result::max_range>(fff, v);
	fixed<32, 12> r0 = res.round<round::truncated>(5);
	float f0 = float_cast<float>(r0);
	fixed<32, 12> r1 = res.round<round::nearest>(5);
	float f1 = float_cast<float>(r1);
	fixed<32, 12> r2 = res.round<round::positive>(5);
	float f2 = float_cast<float>(r2);
	fixed<32, 12> r3 = res.round<round::negative>(5);
	float f3 = float_cast<float>(r3);
	return true;
}

bool val = test1();

static bool test2() {
	for (short i = SHRT_MIN; i < SHRT_MAX; ++i) {
		short s0 = detail::round_impl<short, round::truncated>::round(i, 5);
		short s1 = detail::round_impl<short, round::nearest>::round(i, 5);
		short s2 = detail::round_impl<short, round::positive>::round(i, 5);
		short s3 = detail::round_impl<short, round::negative>::round(i, 5);
	}
	return true;
}

bool vvv = test2();