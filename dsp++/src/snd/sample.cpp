#include <dsp++/snd/sample.h>
#include <dsp++/snd/buffer.h>

#include <boost/lexical_cast.hpp>

#include <limits>
#include <cassert>
#include <cstring>
#include <type_traits>
#include <algorithm>
#include <cctype>

namespace dsp { namespace snd { namespace sample {

type type_of(string_view sf) {
    if (sf.empty()) {
        return type::unknown;
    }
    switch (std::tolower(sf.front())) {
    case 's':
        return type::pcm_signed;
    case 'u':
        return type::pcm_unsigned;
    case 'f':
        return type::ieee_float;
    default:
        return type::unknown;
    }
}

unsigned bit_size_of(string_view sf) {
    if (sf.empty()) {
        return 0;
    }
    int type = std::tolower(sf.front());
    if ('s' != type && 'u' != type && 'f' != type && '_' != type) {
        return 0;
    }
    sf.remove_prefix(1);
    // drop part of format after first dot .
    auto dot = sf.find('.');
    if (dot != sf.npos) {
        sf.remove_suffix(sf.length() - dot);
    }
    try {
        return boost::lexical_cast<unsigned>(sf);
    } catch (boost::bad_lexical_cast&) {
        return 0;
    }
}

namespace {
template<class Int, class Res>
inline void read_pcm_as_float_(const layout& sl, const void* data, Res& res) {
    Int i;
    sl.read_raw_pcm(data, i);
    res = sample::cast<Res>(i);
}

template<class Int, class Float>
inline void write_float_as_pcm_(const layout& sl, Float in, void* out) {
    Int i = sample::cast<Int>(in);
    sl.write_raw_pcm(i, out);
}

template<class Res>
inline void read_pcm_signed_as_float(const layout& sl, const void* data, Res& res) {
    if (sl.container_bytes > 4) {
        read_pcm_as_float_<int64_t>(sl, data, res);
    } else if (sl.container_bytes > 2) {
        read_pcm_as_float_<int32_t>(sl, data, res);
    } else if (sl.container_bytes > 1) {
        read_pcm_as_float_<int16_t>(sl, data, res);
    } else {
        read_pcm_as_float_<int8_t>(sl, data, res);
    }
}

template<class Float>
inline void write_float_as_pcm_signed(const layout& sl, Float f, void* data) {
    if (sl.container_bytes > 4) {
        write_float_as_pcm_<int64_t>(sl, f, data);
    } else if (sl.container_bytes> 2) {
        write_float_as_pcm_<int32_t>(sl, f, data);
    } else if (sl.container_bytes > 1) {
        write_float_as_pcm_<int16_t>(sl, f, data);
    } else {
        write_float_as_pcm_<int8_t>(sl, f, data);
    }
}

template<class Res>
inline void read_pcm_unsigned_as_float(const layout& sl, const void* data, Res& res) {
    if (sl.container_bytes > 4) {
        read_pcm_as_float_<uint64_t>(sl, data, res);
    } else if (sl.container_bytes > 2) {
        read_pcm_as_float_<uint32_t>(sl, data, res);
    } else if (sl.container_bytes > 1) {
        read_pcm_as_float_<uint16_t>(sl, data, res);
    } else {
        read_pcm_as_float_<uint8_t>(sl, data, res);
    }
}

template<class Float>
inline void write_float_as_pcm_unsigned(const layout& sl, Float f, void* data) {
    if (sl.container_bytes > 4) {
        write_float_as_pcm_<uint64_t>(sl, f, data);
    } else if (sl.container_bytes > 2) {
        write_float_as_pcm_<uint32_t>(sl, f, data);
    } else if (sl.container_bytes > 1) {
        write_float_as_pcm_<uint16_t>(sl, f, data);
    } else {
        write_float_as_pcm_<uint8_t>(sl, f, data);
    }
}

template<class Float>
inline void read_ieee_as_float(const layout& sl, const void* data, Float& res) {
    if (4 == sl.container_bytes) {
        float32_t f;
        sl.read_raw_ieee_float(data, f);
        res = static_cast<Float>(f);
    } else if (8 == sl.container_bytes) {
        float64_t f;
        sl.read_raw_ieee_float(data, f);
        res = static_cast<Float>(f);
    }
    else {
        throw std::runtime_error("dsp::snd::layout::read_float() IEEE 754 supports only 32 and 64-bit containers");
    }
}

template<class Float>
inline void write_float_as_ieee(const layout& sl, Float in, void* data) {
    if (4 == sl.container_bytes) {
        float32_t f = sample::cast<float32_t>(in);
        sl.write_raw_ieee_float(f, data);
    } else if (8 == sl.container_bytes) {
        float64_t f = sample::cast<float64_t>(in);
        sl.write_raw_ieee_float(f, data);
    }
    else {
        throw std::runtime_error("dsp::snd::layout::write_float() IEEE 754 supports only 32 and 64-bit containers");
    }
}

template<class Float>
inline void read_sample_as_float(const layout& sl, const void* data, Float& res) {
    switch (sl.type) {
    case type::ieee_float:
        read_ieee_as_float(sl, data, res);
        break;
    case type::pcm_signed:
        read_pcm_signed_as_float(sl, data, res);
        break;
    case type::pcm_unsigned:
        read_pcm_unsigned_as_float(sl, data, res);
        break;
    default:
        throw std::runtime_error("dsp::snd::layout::read_float() unknown sample format");
    }
}

template<class Float>
inline void write_sample_as_float(const layout& sl, Float in, void* data) {
    switch (sl.type) {
    case type::ieee_float:
        write_float_as_ieee(sl, in, data);
        break;
    case type::pcm_signed:
        write_float_as_pcm_signed(sl, in, data);
        break;
    case type::pcm_unsigned:
        write_float_as_pcm_unsigned(sl, in, data);
        break;
    default:
        throw std::runtime_error("dsp::snd::layout::write_float() unknown sample format");
    }
}

}}}

#if !defined(DSPXX_ENDIAN_LITTLE) && !defined(DSPXX_ENDIAN_BIG)

namespace {
static byte_order::label platform_test() {
    int16_t i = 1;
    int8_t buf[2];
    std::memcpy(buf, &i, 2);
    return (buf[0] != 0)
        ? byte_order::little_endian
        : dsp::byte_order::big_endian;
}
}

const byte_order::label byte_order::platform = platform_test();

#endif

namespace snd { namespace sample {

void layout::read_float(const void* data, float& out) const {
    read_sample_as_float(*this, data, out);
}

void layout::read_float(const void* data, double& out) const {
    read_sample_as_float(*this, data, out);
}

void layout::write_float(float in, void* data) const {
    write_sample_as_float(*this, in, data);
}

void layout::write_float(double in, void* data) const {
    write_sample_as_float(*this, in, data);
}

const layout layout::S8{sample::type::pcm_signed, 1};
const layout layout::S16{sample::type::pcm_signed, 2};
const layout layout::S24{sample::type::pcm_signed, 3};
const layout layout::S32{sample::type::pcm_signed, 4};
const layout layout::U8{sample::type::pcm_unsigned, 1};
const layout layout::U16{sample::type::pcm_unsigned, 2};
const layout layout::U24{sample::type::pcm_unsigned, 3};
const layout layout::U32{sample::type::pcm_unsigned, 4};
const layout layout::F32{sample::type::ieee_float, 4};
const layout layout::F64{sample::type::ieee_float, 8};

#if 0

static const char data[] = "\0\1\2\3\4\5\6\7";

static bool test() {
    dsp::snd::layout s8_le(dsp::snd::sample::type::pcm_signed, 1, dsp::snd::byte_order::little_endian);
    long long res;
    s8_le.read_pcm_int_unnormalized(res, &data[0]);
    assert(res == 0);
    s8_le.read_pcm_int_unnormalized(res, &data[1]);
    assert(res == 1);

    dsp::snd::layout s16_le(dsp::snd::sample::type::pcm_signed, 2, dsp::snd::byte_order::little_endian);
    s16_le.read_pcm_int_unnormalized(res, &data[0]);
    assert(res = 256);
    s16_le.read_pcm_int_unnormalized(res, &data[2]);
    assert(res = 3 * 256 + 2);

    dsp::snd::layout s24_le(dsp::snd::sample::type::pcm_signed, 3, dsp::snd::byte_order::little_endian);
    s24_le.read_pcm_int_unnormalized(res, &data[0]);
    assert(res = 1 * 256 + 2 * 65536);
    s24_le.read_pcm_int_unnormalized(res, &data[3]);
    assert(res = 3 + 4 * 256 + 5 * 65536);

    return true;
}


static const bool t = test();

#endif

void convert(
    const layout& sl_in,
    unsigned sample_stride_in,
    const void* in,
    const layout& sl_out,
    unsigned sample_stride_out,
    void* out,
    unsigned length)
{
    const uint8_t* bi = static_cast<const uint8_t*>(in);
    uint8_t* bo = static_cast<uint8_t*>(out);
    unsigned bytes = std::max(sl_in.container_bytes, sl_out.container_bytes);

    for (unsigned i = 0; i < length; ++i, bi += sample_stride_in, bo += sample_stride_out) {
        if (bytes > 4) {
            float64_t f;
            sl_in.read_float(bi, f);
            sl_out.write_float(f, bo);
        } else {
            float32_t f;
            sl_in.read_float(bi, f);
            sl_out.write_float(f, bo);
        };
    }
}

void convert(
    const layout& sl_in,
    const buffer::layout& bl_in,
    const void* in,
    const layout& sl_out,
    const buffer::layout& bl_out,
    void* out,
    unsigned length,
    unsigned channels)
{
    const uint8_t* bi = static_cast<const uint8_t*>(in);
    uint8_t* bo = static_cast<uint8_t*>(out);

    for (unsigned c = 0; c < channels; ++c, bi += bl_in.channel_stride, bo += bl_out.channel_stride) {
        convert(sl_in, bl_in.sample_stride, bi, sl_out, bl_out.sample_stride, bo, length);
    }
}

}}}
