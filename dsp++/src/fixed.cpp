#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>


using namespace dsp::fi;

typedef fixed<16, 0> Q15_t;

Q15_t m = std::numeric_limits<Q15_t>::min();

BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

fixed<8, 3> fff(0.33333f);

Q15_t v(0.33333f);
Q15_t v1(16, raw);
bool ff = (v == v1);


static bool test1() {
	fixed<32, 12> res = multiply_lossless<result::max_range>(fff, v);

	return true;
}

bool val = test1();

