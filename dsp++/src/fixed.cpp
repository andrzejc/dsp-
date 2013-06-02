#include <dsp++/fixed.h>

using namespace dsp;

typedef dsp::detail::fixed_width_impl<0, 15, true> Q15_width;
typedef dsp::detail::fixed_type_impl<Q15_width::width, true>::type Q15_type;
typedef dsp::detail::fixed_promote_impl<31, true>::type Q31p_type;

Q15_type aa;


typedef dsp::fixed<0, 15> Q15_t;
Q15_t::representation_type t;

Q15_t v(15, dsp::raw);
