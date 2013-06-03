#include <dsp++/fixed.h>
#include <boost/static_assert.hpp>
using namespace dsp;

typedef dsp::fixed<16, 0> Q15_t;
BOOST_STATIC_ASSERT(Q15_t::fractional_bits == 15);

dsp::fixed<8, 3> fff;

Q15_t v(15, dsp::raw);
