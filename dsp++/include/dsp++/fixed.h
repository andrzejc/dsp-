
#ifndef DSP_FIXED_H_INCLUDED
#define DSP_FIXED_H_INCLUDED
#pragma once

#include <cstdint>

namespace dsp { 

namespace detail {

	template<bool sel, class T0, class T1> struct select_type_if;
	template<class T0, class T1> struct select_type_if<true, T0, T1> {typedef T0 type;};
	template<class T0, class T1> struct select_type_if<false, T0, T1> {typedef T1 type;};

	template<bool sel, unsigned V0, unsigned V1> struct select_value_if;
	template<unsigned V0, unsigned V1> struct select_value_if<true, V0, V1> {static const unsigned value = V0;};
	template<unsigned V0, unsigned V1> struct select_value_if<false, V0, V1> {static const unsigned value = V1;};


	template<unsigned bits, bool sign> struct fixed_type_impl {
		typedef select_type_if<(bits>64), void, typename fixed_type_impl<bits + 1, sign>::type> type;
		static const unsigned width = select_value_if<(bits>64), bits, fixed_type_impl<bits + 1, sign>::width>::value;
	};

	template<> struct fixed_type_impl<8, true> {typedef std::int8_t type; static const unsigned width = 8;};
	template<> struct fixed_type_impl<8, false> {typedef std::uint8_t type; static const unsigned width = 8;};
	template<> struct fixed_type_impl<16, true> {typedef std::int16_t type; static const unsigned width = 16;};
	template<> struct fixed_type_impl<16, false> {typedef std::uint16_t type; static const unsigned width = 16;};
	template<> struct fixed_type_impl<32, true> {typedef std::int32_t type; static const unsigned width = 32;};
	template<> struct fixed_type_impl<32, false> {typedef std::uint32_t type; static const unsigned width = 32;};
	template<> struct fixed_type_impl<64, true> {typedef std::int64_t type; static const unsigned width = 64;};
	template<> struct fixed_type_impl<64, false> {typedef std::uint64_t type; static const unsigned width = 64;};

	template<unsigned bits, bool sign> struct fixed_promote_impl;
	template<unsigned bits> struct fixed_promote_impl<bits, true> {typedef typename fixed_type_impl<1 + (bits-1) * 2, true>::type type;};
	template<unsigned bits> struct fixed_promote_impl<bits, false> {typedef typename fixed_type_impl<bits * 2, false>::type type;};

	template<unsigned int_bits, unsigned fract_bits, bool sign> struct fixed_width_impl;
	template<unsigned int_bits, unsigned fract_bits> struct fixed_width_impl<int_bits, fract_bits, false> {static const unsigned width = int_bits + fract_bits;};
	template<unsigned int_bits, unsigned fract_bits> struct fixed_width_impl<int_bits, fract_bits, true> {static const unsigned width = int_bits + fract_bits + 1;};

} // namespace detail

//! @brief Holds fixed-point (and other) rounding mode constants, so that we don't have name clashes and can nicely qualify values like round::fastest.
namespace round {
	//! @brief Rounding mode.
	enum mode {
		fastest,		//!< Speed is more important than the choice in value.
		negative,		//!< Round towards negative infinity. This mode is useful in interval arithmetic.
		truncated,		//!< Round towards zero. This mode is useful in implementing integral arithmetic.
		positive,		//!< Round towards positive infinity. This mode is useful in interval arithmetic.
		classic,		//!< Round towards the nearest value, but exactly-half values are rounded towards maximum magnitude. This mode is the standard school algorithm.
		near_even,		//!< Round towards the nearest value, but exactly-half values are rounded towards even values. This mode has more balance than the classic mode.
		near_odd,		//!< Round towards the nearest value, but exactly-half values are rounded towards odd values. This mode has as much balance as the near_even mode, but preserves more information.
	};
} // namespace round

//! @brief Holds fixed-point (and other) overflow mode constants, so that we don't have name clashes and can nicely qualify values like overflow::saturate.
namespace overflow {
	//! @brief Overflow mode.
	enum mode {
		wrap,			//!< Overflowed values will wrap.
		saturate,		//!< If the dynamic value exceeds the range of the variable, assign the nearest representable value.
		exception,		//!< If the dynamic value exceeds the range of the variable, throw an exception of type std::overflow_error.
	};
} // namesapce overflow

//! @brief Tagging type so that we can nicely initialize dsp::fixed objects with dsp::raw param to indicate that the representation value should be used "as is".
struct raw_representation_tag {};
//! @brief Undefined value used to specify that "raw" initializing constructor/assignment should be used.
const raw_representation_tag raw; 


template<unsigned int_bits, unsigned frac_bits, bool sign = true> 
class fixed {
	static const unsigned bits_ = detail::fixed_width_impl<int_bits, frac_bits, sign>::width;
	typedef typename detail::fixed_type_impl<bits_, sign>::type R;
	R v_;
public:
	static const unsigned representation_width = bits_;
	typedef R representation_type;

	explicit fixed(R v, const raw_representation_tag&): v_(v) {}



	representation_type raw() const {return v_;}

private:
};


}  // namespace dsp

#endif // DSP_FIXED_H_INCLUDED
