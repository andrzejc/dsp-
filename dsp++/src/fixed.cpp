#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>
using namespace dsp;

typedef dsp::fixed<16, 0> Q15_t;
BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

dsp::fixed<8, 3> fff;

Q15_t v(15, dsp::raw);
Q15_t v1(16, dsp::raw);
bool ff = (v == v1);
Q15_t f(0.00123f);


Q15_t m = std::numeric_limits<Q15_t>::min();
