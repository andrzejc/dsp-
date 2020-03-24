#include <dsp++/snd/channel.h>

#include <boost/format.hpp>
#include <stdexcept>

namespace dsp { namespace snd { namespace channel {

const layout layout::undefined;
const layout layout::mono = {location::front_left};
const layout layout::stereo = {location::front_left, location::front_right};
const layout layout::s2_1 = layout::stereo | layout{location::lfe};
const layout layout::s3_0_stereo = layout::stereo | layout{location::front_center};
const layout layout::s3_0_surround = layout::stereo | layout{location::back_center};
const layout layout::s4_0_quadro = layout::stereo | layout{location::back_left, location::back_right};
const layout layout::s4_0_surround = layout::s3_0_stereo | layout{location::back_center};
const layout layout::s5_0 = layout::s3_0_stereo | layout{location::back_left, location::back_right};
const layout layout::s5_0_side = layout::s3_0_stereo | layout{location::side_left, location::side_right};
const layout layout::s5_1 = layout::s5_0 | layout{location::lfe};
const layout layout::s5_1_side = layout::s5_0_side | layout{location::lfe};
const layout layout::s6_0 = layout::s5_0 | layout{location::back_center};
const layout layout::s6_0_side = layout::s5_0_side | layout{location::back_center};
const layout layout::s7_0 = layout::s5_0 | layout{location::side_left, location::side_right};
const layout layout::s7_1 = layout::s7_0 | layout{location::lfe};

layout layout::default_for(unsigned channel_count) {
    switch (channel_count) {
    case 1: return mono;
    case 2: return stereo;
    case 3:	return s2_1;
    case 4: return s4_0_quadro;
    case 5:	return s5_0;
    case 6: return s5_1;
    case 7:	return s7_0;
    case 8: return s7_1;
    default:
        return undefined;
    }
}

location layout::at(unsigned index) const {
    unsigned seen = 0;
    for (size_t i = 0; i < size(); ++i) {
        if (base::operator[](i)) {
            if (seen++ == index) {
                return static_cast<location>(i);
            }
        }
    }
    throw std::out_of_range{boost::str(boost::format("index %1% is out of range") % index)};
}

unsigned layout::index_of(location loc) const {
    if (!has(loc)) {
        throw std::out_of_range{boost::str(boost::format("channel at location %1% is not present in the layout") %
            static_cast<size_t>(loc)
        )};
    }
    unsigned seen = 0;
    for (size_t i = 0; i < static_cast<size_t>(loc); ++i) {
        if (base::operator[](i)) {
            ++seen;
        }
    }
    return seen;
}

}}}
