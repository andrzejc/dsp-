#include <dsp++/snd/channel.h>

#include <gtest/gtest.h>

namespace dsp { namespace snd { namespace channel {

TEST(snd_channel_layout, at) {
    EXPECT_EQ(layout::stereo.at(0), location::front_left);
    EXPECT_EQ(layout::stereo.at(1), location::front_right);
    EXPECT_THROW(layout::stereo.at(2), std::out_of_range);
    EXPECT_EQ(layout::s5_1.at(2), location::front_center);
}

TEST(snd_channel_layout, index_of) {
    EXPECT_EQ(layout::stereo.index_of(location::front_left), 0);
    EXPECT_EQ(layout::stereo.index_of(location::front_right), 1);
    EXPECT_THROW(layout::stereo.index_of(location::lfe), std::out_of_range);
    EXPECT_EQ(layout::s5_1.index_of(location::front_center), 2);
}

}}}
