/*!
 * @file dsp++/fixed.h
 * @brief Fixed-point number template and associated stuff.
 */
#ifndef DSP_FIXED_H_INCLUDED
#define DSP_FIXED_H_INCLUDED
#pragma once

#include <dsp++/stdint.h>

#include <limits>

namespace dsp { 

//! @brief Holds fixed-point (and other) rounding mode constants, so that we don't have name clashes and can nicely qualify values like round::fastest.
namespace round {
	//! @brief Rounding mode.
	enum mode {
		fastest = std::round_indeterminate,			//!< Speed is more important than the choice in value.
		negative = std::round_toward_neg_infinity,	//!< Round towards negative infinity. This mode is useful in interval arithmetic.
		truncated = std::round_toward_zero,			//!< Round towards zero. This mode is useful in implementing integral arithmetic.
		positive = std::round_toward_infinity,		//!< Round towards positive infinity. This mode is useful in interval arithmetic.
		classic = std::round_to_nearest,			//!< Round towards the nearest value, but exactly-half values are rounded towards maximum magnitude. This mode is the standard school algorithm.
		near_even, 			//!< Round towards the nearest value, but exactly-half values are rounded towards even values. This mode has more balance than the classic mode.
		near_odd,			//!< Round towards the nearest value, but exactly-half values are rounded towards odd values. This mode has as much balance as the near_even mode, but preserves more information.
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

namespace detail {
	// Implementation details, don't look here (fold down this namespace) ;)
	template<unsigned bits, bool valid = (bits == 8 || bits == 16 || bits == 32 || bits == 64)> struct fixed_word_length_valid;
	template<unsigned bits> struct fixed_word_length_valid<bits, true> {};

	template<unsigned bits, unsigned int_bits, bool sign, bool valid = (int_bits + (sign ? 1 : 0) <= bits)> struct fixed_int_bits_fit_in_word_length;
	template<unsigned bits, unsigned int_bits, bool sign> struct fixed_int_bits_fit_in_word_length<bits, int_bits, sign, true> {};

	template<unsigned bits, unsigned int_bits, bool sign> struct fixed_rep: fixed_word_length_valid<bits>, fixed_int_bits_fit_in_word_length<bits, int_bits, sign> {
		typedef typename select_int<bits, sign>::type type;
		static const unsigned fractional_bits = (bits - int_bits - (sign ? 1 : 0));
	};

	template<class T, unsigned n> struct pow2 {static const T value = 2 * pow2<T, n - 1>::value;};
	template<class T> struct pow2<T, 0> {static const T value = 1;};

//	template<class Res, unsigned frac_bits, class Float> Res float_to_fixed

} // namespace detail

/*!
 * @brief Implementation of fixed-point number with parametrized word length, number of integer and fractional bits and signedness.
 * @tparam WordLength total number of bits the number is stored in. Must be one of: 8, 16, 32, 64.
 * @tparam IntBits number of integer bits. Number of fractional bits is inferred from @p WordLength, @p IntBits and @p sign.
 * @tparam sign specifies whether this is signed or unsigned number.
 * @pre Only word lengths that can be represented by built-in integer types are allowed. This is enforced by detail::fixed_word_length_valid compile-time check.
 * @pre (IntBits <= WordLength - (sign ? 1 : 0)) This is enforced by detail::fixed_int_bits_fit_in_word_length compile-time check.
 */
template<unsigned WordLength, unsigned IntBits, bool sign = true>
class fixed {
	typedef detail::fixed_rep<WordLength, IntBits, sign> F;
	typedef typename F::type R;
	R v_;
public:
	static const unsigned word_length = WordLength;					//!< Well... word length.
	static const unsigned integer_bits = IntBits;					//!< Number of integer bits.
	static const unsigned fractional_bits = F::fractional_bits;		//!< Number of fractional bits.
	typedef R representation_type;									//!< Type of underlying integer representation.

	/*!@brief Default constructor, initializes fixed point number to 0. */
	fixed(): v_(R()) {}
	/*!@brief Trivial copy constructor from the same type, simply copy the representation value.
	 * @param[in] rhs fixed-point number to initialize this to. */
	fixed(const fixed& rhs): v_(rhs.v_) {}
	/*!@brief Initialize this fixed-point number with raw integer representation. Useful for interacting with outside-world
	 * (e.g. using data generated by MATLAB).
	 * @param[in] v the value to initialize representation with.
	 */
	explicit fixed(R v, const raw_representation_tag&): v_(v) {}

	//!@todo add dsp::round::mode param and dsp::overflow::mode param, currently both are default (truncated/wrap)
	explicit fixed(float v): v_(static_cast<R>((1ull << fractional_bits) * v)) {}
	explicit fixed(double v): v_(static_cast<R>((1ull << fractional_bits) * v)) {}
	explicit fixed(long double v): v_(static_cast<R>((1ull << fractional_bits) * v)) {}


	/*!@brief Obtain raw integer representation.
	 * @return raw integer representation value. */
	representation_type raw() const {return v_;}
	/*!@brief Trivial copy constructor from the same type, simply copy the representation value.
	 * @param[in] rhs fixed-point number copy.
	 * @return *this. */
	fixed& operator=(const fixed& rhs) {v_ = rhs.v_; return *this;}

	friend bool operator==(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ == rhs.v_;}
	friend bool operator!=(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ != rhs.v_;}
	friend bool operator<(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ < rhs.v_;}
	friend bool operator<=(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ <= rhs.v_;}
	friend bool operator>(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ > rhs.v_;}
	friend bool operator>=(const fixed<WordLength, IntBits, sign>& lhs, const fixed<WordLength, IntBits, sign>& rhs) {return lhs.v_ > rhs.v_;}

private:
};

}  // namespace dsp

namespace std { // specializing std::numeric_limits for dsp::fixed

template<unsigned WordLength, unsigned IntBits, bool sign>
class numeric_limits<dsp::fixed<WordLength, IntBits, sign> > {
	typedef dsp::fixed<WordLength, IntBits, sign> F;
	typedef typename F::representation_type R;
	typedef std::numeric_limits<R> NL;
public:

    static const bool is_specialized = true;

    static F min() throw() {return F(1, dsp::raw);}
    static F max() throw() {return F(NL::max(), dsp::raw);}
    static F lowest() throw() {return F(NL::min(), dsp::raw);}

    static const int digits = static_cast<int>(IntBits);
    static const int digits10 = (int)(digits * 301L / 1000);
    static const bool is_signed = sign;
    static const bool is_integer = false;
    static const bool is_exact = true;
    static const int radix = 2;

    static F epsilon() throw() {return F(1, dsp::raw);}
    static F round_error() throw() {return F(.5f);}

    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool has_infinity = false;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
	static const float_denorm_style has_denorm = denorm_absent;
	static const bool has_denorm_loss = false;

    static F infinity() throw() { return F(); }
    static F quiet_NaN() throw() { return F(); }
    static F signaling_NaN() throw() { return F(); }
    static F denorm_min() throw() { return min(); }

    static const bool is_iec559 = false;
    static const bool is_bounded = true;
    static const bool is_modulo = false;

    static const bool traps = false;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero; // this is the default rounding mode used by parameterless operators
};

} // namespace std

#endif // DSP_FIXED_H_INCLUDED
