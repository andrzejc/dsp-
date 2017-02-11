/*!
 * @file dsp++/snd/sample.h
 * @brief Sample type definitions & conversion routines
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_SND_SAMPLE_H_INCLUDED
#define DSP_SND_SAMPLE_H_INCLUDED

#include <dsp++/export.h>
#include <dsp++/snd/format.h>
#include <dsp++/stdint.h>
#include <dsp++/float.h>
#include <dsp++/platform.h>
#include <dsp++/intmath.h>
#include <dsp++/compat.h>

#include <limits>
#include <stdexcept>
#include <type_traits>

namespace dsp { namespace snd {

namespace detail {

template<class UInt>
struct shift_right8_impl {
	UInt shift(UInt in)
	{
		return in >> 8;
	}
};

template<>
struct shift_right8_impl<uint8_t> {
	uint8_t shift(uint8_t /* in */)
	{
		return 0;
	}
};
}


//! @brief Describes memory organization of a single sample.
struct sample_layout {

	sample::type::label type;		//!< Type of sample data.
	unsigned container_bytes;		//!< Number of bytes the sample is stored in.
	unsigned significant_bits;		//!< Number of bits carrying significant data.
	unsigned lsb_padding;			//!< Number of padding bits to the right of LSB
	byte_order::label byte_order;	//!< Sample byte order

	//! @brief Construct sample_layout object with specified configuration.
	//! @param[in] t sample type (coding)
	//! @param[in] bytes size of sample container in bytes
	//! @param[in] bo byte order (big/little)
	//! @param[in] sig number of significant bits in sample (if 0/default: 8 * bytes, entire containter is used)
	//! @param[in] lsb_pad number of padding bits to the left of LSB
	sample_layout(sample::type::label t, unsigned bytes, byte_order::label bo, unsigned sig = 0, unsigned lsb_pad = 0)
	 :	type(t)
	 ,	container_bytes(bytes)
	 ,	significant_bits(0 == sig ? bytes * 8 : sig)
	 ,	lsb_padding(lsb_pad)
	 ,	byte_order(bo)
	{}

	sample_layout()
	 : type(sample::type::unknown)
	 , container_bytes(0)
	 , significant_bits(0)
	 , lsb_padding(0)
	 , byte_order(byte_order::little_endian)
	{}

	//! @brief Read integer (LPCM) sample from byte buffer described by this layout into integer variable.
	//! The type of sample data must be sample::type::pcm_signed for signed Int type or sample::type::pcm_unsigned
	//! for unsigned Int type.
	//! @tparam Int type of output variable
	//! @param[in] data pointer to byte buffer described by this sample_layout.
	//! @param[out] out variable which will receive read sample, full range of integer type will be used (sample will be normalized).
	template<class Int>
	void read_pcm(const void* data, Int& out) const {
		static_assert(std::is_integral<Int>::value, "Int must ba an integeral type");
		if (!((sample::type::pcm_signed == type && std::numeric_limits<Int>::is_signed) ||
			(sample::type::pcm_unsigned == type && !std::numeric_limits<Int>::is_signed)))
			throw std::invalid_argument("dsp::snd::sample_layout::read_pcm() requires compatible "
			                            "sample::type::pcm_(un)signed");
		out = static_cast<Int>(read_bits<typename unsigned_of<Int>::type>(data));
	}

	template<class Int>
	void read_pcm_right_aligned(const void* data, Int& out) const {
		read_pcm(data, out);
		out >>= (sizeof(Int) * 8 - significant_bits);	// perform arithmetic shift of output so that sign is preserved
														// and LSB is right-aligned
	}

	//! @brief Read floating-point sample from byte buffer described by this layout into float variable.
	//! The type of sample data must be sample::type::ieee_float.
	//! @tparam Int type of output variable
	//! @param[in] data pointer to byte buffer described by this sample_layout.
	//! @param[out] out variable which will receive read sample, full range of integer type will be used (sample will be normalized).
	template<class Float>
	void read_ieee_float(const void* data, Float& out) const {
		static_assert(std::is_floating_point<Float>::value, "Float must ba a floating-point type");
		if (sample::type::ieee_float != type)
			throw std::invalid_argument("dsp::snd::sample_layout::read_ieee_float() requires sample::type::ieee_float");
		typedef typename select_int<sizeof(Float) * 8, false>::type uint;
		union {
			Float f;
			uint u;
		} v;
		v.u = read_bits<uint>(data);
		out = v.f;
	}

	template<class Int>
	void write_pcm(Int in, void* data) const {
		static_assert(std::is_integral<Int>::value, "Int must ba an integeral type");
		if (!((sample::type::pcm_signed == type && std::numeric_limits<Int>::is_signed) ||
			(sample::type::pcm_unsigned == type && !std::numeric_limits<Int>::is_signed)))
			throw std::invalid_argument("dsp::snd::sample_layout::write_pcm() requires compatible sample::type::pcm_(un)signed");
		write_bits(static_cast<typename unsigned_of<Int>::type>(in), data);
	}

	template<class Int>
	void write_pcm_right_aligned(Int in, void* data) const {
		in <<= (sizeof(Int) * 8 - significant_bits);
		write_pcm(in, data);
	}

	template<class Float>
	void write_ieee_float(Float in, void* out) const {
		static_assert(std::is_floating_point<Float>::value, "Float must ba a floating-point type");
		if (sample::type::ieee_float != type)
			throw std::invalid_argument("dsp::snd::sample_layout::write_ieee_float() requires sample::type::ieee_float");
		typedef typename select_int<sizeof(Float) * 8, false>::type uint;
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

private:
	template<class UInt>
	UInt read_bits(const void* data) const {
		const uint8_t* b = static_cast<const uint8_t*>(data);
		UInt ures = 0;
		int shift;
		int shift_step;
		if (byte_order::little_endian == this->byte_order) {
			shift = 0;
			shift_step = 8;
		}
		else {
			shift = 8 * (container_bytes - 1);
			shift_step = -8;
		}
		for (unsigned i = 0; i < container_bytes; ++i, ++b, shift += shift_step)
			ures |= (static_cast<UInt>(*b) << shift);

		ures >>= lsb_padding;	// cancel LSB padding bits
		ures <<= (sizeof(UInt) * 8 - significant_bits);	// normalize to take full use of output type, put sign bit in
														// place and cancel MSB padding bits
		return ures;
	}

	template<class UInt>
	void write_bits(UInt bits, void* data) const {
		uint8_t* b;
		detail::shift_right8_impl<UInt> shr8;
		bits >>= 8 * (sizeof(UInt) - container_bytes);
		if (byte_order::little_endian == byte_order) {
			b = static_cast<uint8_t*>(data);
			for (unsigned i = 0; i < container_bytes; ++i, ++b) {
				*b = static_cast<uint8_t>(bits);
				bits = shr8.shift(bits);
			}
		}
		else {
			b = static_cast<uint8_t*>(data) + container_bytes - 1;
			for (unsigned i = 0; i < container_bytes; ++i, --b) {
				*b = static_cast<uint8_t>(bits);
				bits = shr8.shift(bits);
			}
		}
	}
};

namespace detail {
	template<class In, class Out,
		bool InSigned = std::numeric_limits<In>::is_signed, bool OutSigned = std::numeric_limits<Out>::is_signed,
		bool InFloat = !std::numeric_limits<In>::is_integer, bool OutFloat = !std::numeric_limits<Out>::is_integer>
	struct sample_cast_impl;

	template<class In, class Out> // float -> float
	struct sample_cast_impl<In, Out, true, true, true, true> {
		static Out cast(In in) {return static_cast<Out>(in);}
	};

	template<class In, class Out> // signed int -> float
	struct sample_cast_impl<In, Out, true, true, false, true> {
		static Out cast(In in) {return in / -static_cast<Out>(std::numeric_limits<In>::min());}
	};

	template<class In, class Out> // unsigned int -> float
	struct sample_cast_impl<In, Out, false, true, false, true> {
		static Out cast(In in) {return in / (static_cast<Out>(std::numeric_limits<In>::max()) + Out(1.)) - Out(.5);}
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
			return rint<Out, rounding::nearest,	overflow::saturate>((in + In(.5)) * (static_cast<In>(std::numeric_limits<Out>::max()) + In(1.)));
		}
	};

};

template<class Out, class In>
Out sample_cast(In in) {
	return detail::sample_cast_impl<In, Out>::cast(in);
}

}}

#endif /* DSP_SND_SAMPLE_H_INCLUDED */
