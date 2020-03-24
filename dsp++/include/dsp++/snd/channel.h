/*!
 * @file dsp++/snd/channel.h
 * @brief Audio stream channel layout description.
 */
#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>

#include <cstddef>
#include <bitset>
#include <initializer_list>
#include <functional>

namespace dsp { namespace snd {
//! Constants, functions, etc. related to labeling channels in a multichannel sound file formats.
namespace channel {

//! @brief Speaker location indices.
//! This is ordered according to USB Audio class definition, which is a native representation both in WAVE files
//! and in CoreAudio.
//! @see http://www.usb.org/developers/devclass_docs/audio10.pdf
//! @see http://msdn.microsoft.com/en-us/windows/hardware/gg463006
//! @see https://developer.apple.com/library/mac/#qa/qa1638/_index.html
enum class location {
    front_left,
    front_right,
    front_center,
    lfe,
    back_left,
    back_right,
    front_left_center,
    front_right_center,
    back_center,
    side_left,
    side_right,
    top_center,
    top_front_left,
    top_front_center,
    top_front_right,
    top_back_left,
    top_back_center,
    top_back_right,

    COUNT //!< Number of location labels, always last.
};

class DSPXX_API layout: private std::bitset<static_cast<size_t>(location::COUNT)> {
    using base = std::bitset<static_cast<size_t>(location::COUNT)>;

public:
    using base::reference;
    constexpr layout() noexcept {}
    constexpr explicit layout(unsigned long long mask) noexcept: base{mask} {}
    layout(const std::initializer_list<location>& locs) {
        for(auto loc: locs) {
            set(loc);
        }
    }

    constexpr bool operator[](location loc) const {
        return base::operator[](static_cast<std::size_t>(loc));
    }
    reference operator[](location loc) {
        return base::operator[](static_cast<std::size_t>(loc));
    }
    constexpr bool has(location loc) const {
        return operator[](loc);
    }
    using base::any;
    using base::none;
    unsigned count() const noexcept {
        return static_cast<unsigned>(base::count());
    }
    unsigned long long mask() const {
        return base::to_ullong();
    }

    //! @throw std::out_of_range if index >= count().
    location at(unsigned index) const;
    //! @throw std::out_of_range if @p loc is not present in the layout
    unsigned index_of(location loc) const;

    layout& set(location loc, bool present = true) {
        base::set(static_cast<size_t>(loc), present);
        return *this;
    }
    layout& reset(location loc) {
        base::reset(static_cast<size_t>(loc));
        return *this;
    }
    layout& reset() noexcept {
        base::reset();
        return *this;
    }

    layout& operator&=(const layout& oth) noexcept {
        static_cast<base&>(*this) &= static_cast<const base&>(oth);
        return *this;
    }

    layout& operator|=(const layout& oth) noexcept {
        static_cast<base&>(*this) |= static_cast<const base&>(oth);
        return *this;
    }
    bool operator==(const layout& rhs) const noexcept {
        return base::operator==(rhs);
    }
    bool operator!=(const layout& rhs) const noexcept {
        return base::operator!=(rhs);
    }

    static const layout undefined;
    static const layout mono;           // {location::front_left}
    static const layout stereo;         // {location::front_left, location::front_right}
    static const layout s2_1;           // stereo | {location::lfe}
    static const layout s3_0_stereo;    // stereo | {location::front_center}
    static const layout s3_0_surround;  // stereo | {location::back_center}
    static const layout s4_0_quadro;    // stereo | {location::back_left, location::back_right}
    static const layout s4_0_surround;  // s3_0_stereo | {location::back_center}
    static const layout s5_0;           // s3_0_stereo | {location::back_left, location::back_right}
    static const layout s5_0_side;      // s3_0_stereo | {location::side_left, location::side_right}
    static const layout s5_1;           // s5_0 | {location::lfe}
    static const layout s5_1_side;      // s5_0_side | {location::lfe}
    static const layout s6_0;           // s5_0 | {location::back_center}
    static const layout s6_0_side;      // s5_0_side | {location::back_center}
    static const layout s7_0;           // s5_0 | {location::side_left, location::side_right}
    static const layout s7_1;           // s7_0 | {location::lfe},

    friend struct std::hash<layout>;

    //! @param[in] channel_count number of channels.
    //! @return "default" (most typical) speaker layout for given channel count.
    static layout default_for(unsigned channel_count);
};

inline layout operator|(const layout& lhs, const layout& rhs) noexcept {
    layout res = lhs;
    res |= rhs;
    return res;
}

inline layout operator&(const layout& lhs, const layout& rhs) noexcept {
    layout res = lhs;
    res &= rhs;
    return res;
}
} // namespace channel
}} // namespace dsp::snd

namespace std {
template<>
struct hash<dsp::snd::channel::layout> {
    size_t operator()(const dsp::snd::channel::layout& l) const {
        return hash<dsp::snd::channel::layout::base>{}(l);
    }
};
}
