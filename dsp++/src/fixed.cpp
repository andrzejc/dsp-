#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>
#include <climits>

using namespace dsp::fi;

typedef fixed<16, 0> Q15_t;

Q15_t m = std::numeric_limits<Q15_t>::min();

BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

fixed<8, 3> fff(-0.33333f);

Q15_t v(0.33333);
Q15_t v1(2048, raw);
bool ff = (v == v1);


static bool test1() {
	fixed<32, 12> res = multiply_lossless<result::max_range>(fff, v);
	fixed<32, 4> res1 = multiply_lossless<result::max_precision>(fff, v);
	float ff = float_cast<float>(res);
	float rr = float_cast<float>(res1);

	fixed<32, 12> r0 = res.round<dsp::rounding::fastest>(5);
	float f0 = float_cast<float>(r0);
	fixed<32, 12> r1 = res.round<dsp::rounding::nearest>(5);
	float f1 = float_cast<float>(r1);
	fixed<32, 12> r2 = res.round<dsp::rounding::positive>(5);
	float f2 = float_cast<float>(r2);
	fixed<32, 12> r3 = res.round<dsp::rounding::negative>(5);
	float f3 = float_cast<float>(r3);

	float fv1 = float_cast<float>(v1);
	fv1 *= 0.33333;

	fixed<16, 0> q = v * v1;
	float fq = float_cast<float>(q);
	return true;
}

bool val = test1();

static bool test2() {
//	for (short i = SHRT_MIN; i < SHRT_MAX; ++i) {
//		short s0 = dsp::round<dsp::rounding::truncated, dsp::overflow::fastest>(i, 5);
//		short s1 = dsp::round<dsp::rounding::nearest, dsp::overflow::exception>(i, 5);
//		short s2 = dsp::round<dsp::rounding::positive, dsp::overflow::saturate>(i, 5);
//		short s3 = dsp::round<dsp::rounding::negative, dsp::overflow::wrap>(i, 5);
//	}
	return true;
}

bool vvv = test2();
