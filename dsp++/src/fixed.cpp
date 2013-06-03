#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>

using namespace dsp;

typedef dsp::fixed<16, 0> Q15_t;
BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

dsp::fixed<8, 3> fff(0.33333f);

Q15_t v(0.33333f);
Q15_t v1(16, dsp::raw);
bool ff = (v == v1);


static bool test1() {
	dsp::fixed<32, 12> res = dsp::multiply_lossless(fff, v);

	return true;
}

bool val = test1();

Q15_t m = std::numeric_limits<Q15_t>::min();

