
#ifndef DSP_FIXED_H_INCLUDED
#define DSP_FIXED_H_INCLUDED
#pragma once

#include <dsp++/stdint.h>

namespace dsp { 

namespace detail {

	template<bool sel, class T0, class T1> struct select_type_if;
	template<class T0, class T1> struct select_type_if<true, T0, T1> {typedef T0 type;};
	template<class T0, class T1> struct select_type_if<false, T0, T1> {typedef T1 type;};

	template<unsigned bits, bool valid = (bits == 8 || bits == 16 || bits == 32 || bits == 64)> struct fixed_word_length_valid;
	template<unsigned bits> struct fixed_word_length_valid<bits, true> {};

	template<unsigned bits, unsigned int_bits, bool sign, bool valid = (int_bits + (sign ? 1 : 0) <= bits)> struct int_bits_fit_in_word_length;
	template<unsigned bits, unsigned int_bits, bool sign> struct int_bits_fit_in_word_length<bits, int_bits, sign, true> {};

	template<unsigned bits, unsigned int_bits, bool sign> struct fixed_rep: fixed_word_length_valid<bits>, int_bits_fit_in_word_length<bits, int_bits, sign> {
		typedef typename select_int<bits, sign>::type type;
		static const unsigned fractional_bits = (bits - int_bits - (sign ? 1 : 0));
	};

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
} // namespace overflow

//! @brief Tagging type so that we can nicely initialize dsp::fixed objects with dsp::raw param to indicate that the representation value should be used "as is".
struct raw_representation_tag {};
//! @brief Undefined value used to specify that "raw" initializing constructor/assignment should be used.
const raw_representation_tag raw = {};


template<unsigned WordLength, unsigned IntBits, bool sign = true>
class fixed {
	typedef detail::fixed_rep<WordLength, IntBits, sign> F;
	typedef typename F::type R;
	R v_;
public:
	static const unsigned word_length = WordLength;
	static const unsigned integer_bits = IntBits;
	static const unsigned fractional_bits = F::fractional_bits;
	typedef R representation_type;

	fixed(): v_(R()) {}
	explicit fixed(R v, const raw_representation_tag&): v_(v) {}

//	template<unsigned oth_int, unsigned oth_frac, bool oth_sign>
//	fixed<oth_int+int_bits, oth_frac+frac_bits, !(sign || oth_sign)> mul(fixed<oth_int, oth_frac, oth_sign> val);

	representation_type raw() const {return v_;}

private:
};


}  // namespace dsp

#endif // DSP_FIXED_H_INCLUDED
