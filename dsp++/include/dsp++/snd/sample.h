/*!
 * @file dsp++/snd/sample.h
 * @brief Sample type definitions & conversion routines
 */
#pragma once
#include <dsp++/export.h>
#include <dsp++/types.h>
#include <dsp++/stdint.h>
#include <dsp++/float.h>
#include <dsp++/platform.h>
#include <dsp++/intmath.h>

#include <limits>
#include <stdexcept>
#include <type_traits>

namespace dsp { namespace snd { namespace sample {

/// @brief Labels of audio sample formats.
namespace format {
constexpr char U8[] =  "U8";   ///< Unsigned 8-bit integer with offset of 128 linear PCM.
constexpr char U16[] = "U16";  ///< Unsigned 16-bit integer PCM.
constexpr char U24[] = "U24";  ///< Unsigned 24-bit integer PCM.
constexpr char U32[] = "U32";  ///< Unsigned 32-bit integer PCM.
constexpr char S8[] =  "S8";   ///< Signed 8-bit integer, linear PCM.
constexpr char S16[] = "S16";  ///< Signed 16-bit integer linear PCM.
constexpr char S24[] = "S24";  ///< Signed 24-bit integer (packed) linear PCM.
constexpr char S32[] = "S32";  ///< Signed 32-bit integer linear PCM.
constexpr char F32[] = "F32";  ///< Floating-point 32-bit (with a non-overdriving range of [-1.0, 1.0]).
constexpr char F64[] = "F64";  ///< Floating-point 64-bit (with a non-overdriving range of [-1.0, 1.0]).
} // namespace format

constexpr unsigned bit_size_unknown = 0;

/// @param[in] format_spec format label to parse for sample bit size
/// @return bit size of sample in given format.
DSPXX_API unsigned bit_size_of(string_view format_spec);

/// @brief Sample type labels as returned by dsp::snd::sample::type_of().
enum class type {
    unknown,
    pcm_unsigned,
    pcm_signed,
    ieee_float,
};

/// @param[in] format_spec format description to parse for sample type.
/// @return sample type of given format.
DSPXX_API type type_of(string_view format_spec);

namespace detail {

template<class In, class Out,
    bool InSigned = std::numeric_limits<In>::is_signed, bool OutSigned = std::numeric_limits<Out>::is_signed,
    bool InFloat = !std::numeric_limits<In>::is_integer, bool OutFloat = !std::numeric_limits<Out>::is_integer>
struct sample_cast_impl;

template<class In, class Out> // float -> float
struct sample_cast_impl<In, Out, true, true, true, true> {
    static Out cast(In in) {
        return static_cast<Out>(in);
    }
};

template<class In, class Out> // signed int -> float
struct sample_cast_impl<In, Out, true, true, false, true> {
    static Out cast(In in) {
        return in / -static_cast<Out>(std::numeric_limits<In>::min());
    }
};

template<class In, class Out> // unsigned int -> float
struct sample_cast_impl<In, Out, false, true, false, true> {
    static Out cast(In in) {
        return Out{2} * in / (static_cast<Out>(std::numeric_limits<In>::max()) + Out{1}) - Out{1};
    }
};

template<class In, class Out> // float -> signed int
struct sample_cast_impl<In, Out, true, true, true, false> {
    static Out cast(In in) {
        return rint<Out, rounding::nearest,	overflow::saturate>(in * -static_cast<In>(std::numeric_limits<Out>::min()));
    }
};

template<class In, class Out> // float -> unsigned int
struct sample_cast_impl<In, Out, true, false, true, false> {
    static Out cast(In in) {
        return rint<Out, rounding::nearest,	overflow::saturate>((in + In{1}) * (static_cast<In>(std::numeric_limits<Out>::max()) + In{1.}) / In{2});
    }
};

}

template<class Out, class In>
inline Out cast(In in) {
    return detail::sample_cast_impl<In, Out>::cast(in);
}

namespace detail {
template<class UInt>
struct shift_right8_impl {
    static constexpr UInt shift(UInt in) {
        return in >> 8;
    }
};

template<>
struct shift_right8_impl<uint8_t> {
    static constexpr uint8_t shift(uint8_t /* in */) {
        return 0;
    }
};
}

/// @brief Describes memory organization of a single sample.
struct DSPXX_API layout {
    sample::type type = sample::type::unknown;                                  ///< Type of sample data.
    unsigned container_bytes = 0;                               ///< Number of bytes the sample is stored in.
    unsigned significant_bits = 0;                              ///< Number of bits carrying significant data.
    unsigned lsb_padding = 0;                                   ///< Number of padding bits to the right of LSB
    byte_order::label byte_order = byte_order::little_endian;   ///< Sample byte order

    /// @brief Construct layout object with specified configuration.
    /// @param[in] t sample type (coding)
    /// @param[in] bytes size of sample container in bytes
    /// @param[in] bo byte order (big/little)
    /// @param[in] sig number of significant bits in sample (if 0/default: 8 * bytes, entire containter is used)
    /// @param[in] lsb_pad number of padding bits to the left of LSB
    constexpr layout(
        sample::type t,
        unsigned bytes,
        byte_order::label bo = byte_order::platform,
        unsigned sig = 0,
        unsigned lsb_pad = 0
    ):
        type(t),
        container_bytes(bytes),
        significant_bits(0 == sig ? bytes * 8 - lsb_pad : sig),
        lsb_padding(lsb_pad),
        byte_order(bo)
    {}

    /// @brief Read integer (LPCM) sample from byte buffer described by this layout into integer variable.
    /// The type of sample data must be sample::type::pcm_signed for signed Int type or sample::type::pcm_unsigned
    /// for unsigned Int type.
    /// @tparam Int type of output variable
    /// @param[in] data pointer to byte buffer described by this layout.
    /// @param[out] out variable which will receive read sample, full range of integer type will be used (sample will be normalized).
    template<class Int>
    void read_raw_pcm(const void* data, Int& out) const {
        static_assert(std::is_integral<Int>::value, "Int must ba an integeral type");
        if (!( (sample::type::pcm_signed == type && std::numeric_limits<Int>::is_signed)
               || (sample::type::pcm_unsigned == type && !std::numeric_limits<Int>::is_signed))
        ) {
            throw std::invalid_argument("dsp::snd::sample::layout::read_raw_pcm() requires compatible "
                                        "sample::type::pcm_(un)signed");
        }
        out = static_cast<Int>(read_bits<unsigned_of_t<Int>>(data));
    }

    template<class Int>
    void read_raw_pcm_right_aligned(const void* data, Int& out) const {
        read_raw_pcm(data, out);
        out >>= (sizeof(Int) * 8 - significant_bits);   // perform arithmetic shift of output so that sign is preserved
                                                        // and LSB is right-aligned
    }

    /// @brief Read floating-point sample from byte buffer described by this layout into float variable.
    /// The type of sample data must be sample::type::ieee_float.
    /// @tparam Int type of output variable
    /// @param[in] data pointer to byte buffer described by this layout.
    /// @param[out] out variable which will receive read sample, full range of integer type will be used (sample will be normalized).
    template<class Float>
    void read_raw_ieee_float(const void* data, Float& out) const {
        static_assert(std::is_floating_point<Float>::value, "Float must ba a floating-point type");
        if (sample::type::ieee_float != type) {
            throw std::invalid_argument("dsp::snd::sample::layout::read_raw_ieee_float() requires sample::type::ieee_float");
        }
        using uint = select_int_t<sizeof(Float) * 8, false>;
        union {
            Float f;
            uint u;
        } v;
        v.u = read_bits<uint>(data);
        out = v.f;
    }

    template<class Int>
    void write_raw_pcm(Int in, void* data) const {
        static_assert(std::is_integral<Int>::value, "Int must ba an integeral type");
        if (!((sample::type::pcm_signed == type && std::numeric_limits<Int>::is_signed) ||
            (sample::type::pcm_unsigned == type && !std::numeric_limits<Int>::is_signed)))
        {
            throw std::invalid_argument("dsp::snd::sample::layout::write_raw_pcm() requires compatible sample::type::pcm_(un)signed");
        }
        write_bits(static_cast<unsigned_of_t<Int>>(in), data);
    }

    template<class Int>
    void write_raw_pcm_right_aligned(Int in, void* data) const {
        in <<= (sizeof(Int) * 8 - significant_bits);
        write_raw_pcm(in, data);
    }

    template<class Float>
    void write_raw_ieee_float(Float in, void* out) const {
        static_assert(std::is_floating_point<Float>::value, "Float must ba a floating-point type");
        if (sample::type::ieee_float != type) {
            throw std::invalid_argument("dsp::snd::sample::layout::write_raw_ieee_float() requires sample::type::ieee_float");
        }
        using uint = select_int_t<sizeof(Float) * 8, false>;
        union {
            Float f;
            uint u;
        } v;
        v.f = in;
        write_bits(v.u, out);
    }

    void read_float(const void* in, float& out) const;
    void read_float(const void* in, double& out) const;

    void write_float(float in, void* out) const;
    void write_float(double in, void* out) const;

    static const layout S8;
    static const layout S16;
    static const layout S24;
    static const layout S32;
    static const layout U8;
    static const layout U16;
    static const layout U24;
    static const layout U32;
    static const layout F32;
    static const layout F64;

    template<typename Sample>
    static constexpr layout of() {
        static_assert(std::is_floating_point<Sample>::value || std::is_integral<Sample>::value, "of works only for numeric types");
        return layout{
            std::is_floating_point<Sample>::value
                ? sample::type::ieee_float
                : std::is_signed<Sample>::value
                    ? sample::type::pcm_signed
                    : sample::type::pcm_unsigned,
            sizeof(Sample)
        };
    }

private:
    template<class UInt>
    UInt read_bits(const void* data) const {
        static_assert(std::is_integral<UInt>::value && !std::is_signed<UInt>::value, "result must be unsigned type");
        const uint8_t* b = static_cast<const uint8_t*>(data);
        UInt ures = {};
        int shift;
        int shift_step;
        if (byte_order::little_endian == this->byte_order) {
            shift = 0;
            shift_step = 8;
        } else {
            shift = 8 * (container_bytes - 1);
            shift_step = -8;
        }
        for (unsigned i = 0; i < container_bytes; ++i, ++b, shift += shift_step) {
            ures |= (static_cast<UInt>(*b) << shift);
        }

        ures >>= lsb_padding;	// cancel LSB padding bits
        ures <<= (sizeof(UInt) * 8 - significant_bits);	// normalize to take full use of output type, put sign bit in
                                                        // place and cancel MSB padding bits
        return ures;
    }

    template<class UInt>
    void write_bits(UInt bits, void* data) const {
        static_assert(std::is_integral<UInt>::value && !std::is_signed<UInt>::value, "bits value must be of unsigned type");
        bits >>= 8 * (sizeof(UInt) - container_bytes);
        if (byte_order::little_endian == byte_order) {
            uint8_t* b = static_cast<uint8_t*>(data);
            for (unsigned i = 0; i < container_bytes; ++i, ++b) {
                *b = static_cast<uint8_t>(bits);
                bits = detail::shift_right8_impl<UInt>::shift(bits);
            }
        } else {
            uint8_t* b = static_cast<uint8_t*>(data) + container_bytes - 1;
            for (unsigned i = 0; i < container_bytes; ++i, --b) {
                *b = static_cast<uint8_t>(bits);
                bits = detail::shift_right8_impl<UInt>::shift(bits);
            }
        }
    }
};

template<typename Sample> struct format_of;
template<>
struct format_of<float32_t> { constexpr static auto value = format::F32; };
template<>
struct format_of<float64_t> { constexpr static auto value = format::F64; };
template<>
struct format_of<int8_t> { constexpr static auto value = format::S8; };
template<>
struct format_of<int16_t> { constexpr static auto value = format::S16; };
template<>
struct format_of<int32_t> { constexpr static auto value = format::S32; };
template<>
struct format_of<uint8_t> { constexpr static auto value = format::U8; };
template<>
struct format_of<uint16_t> { constexpr static auto value = format::U16; };
template<>
struct format_of<uint32_t> { constexpr static auto value = format::U32; };

template<typename Sample>
constexpr auto format_of_v = format_of<Sample>::value;
}  // namespace sample

namespace buffer {
struct layout;
}

namespace sample {

DSPXX_API void convert(
    const layout& sl_in,
    unsigned sample_stride_in,
    const void* in,
    const layout& sl_out,
    unsigned sample_stride_out,
    void* out,
    unsigned length
);

DSPXX_API void convert(
    const layout& sl_in,
    const buffer::layout& bl_in,
    const void* in,
    const layout& sl_out,
    const buffer::layout& bl_out,
    void* out,
    unsigned length,
    unsigned channels
);

}}}
