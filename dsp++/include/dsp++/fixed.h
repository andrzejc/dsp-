/*!
 * @file dsp++/fixed.h
 * @brief Fixed-point number template and associated stuff.
 */
#ifndef DSP_FIXED_H_INCLUDED
#define DSP_FIXED_H_INCLUDED
#pragma once

#include <dsp++/stdint.h>
#include <dsp++/intmath.h>

#include <limits>
#include <cmath>
#include <functional>

namespace dsp { namespace fi {


//! @brief Fixed-point rounding function object. Rounds the argument passed to operator() after specified fractional bit position (use 0 to round at integers).
//! @tparam RoundMode Rounding mode to use.
//! @tparam OverflowMode The way this object handles the overflow which may occur during rounding.
template<int WordLength, int IntBits, bool IsSigned, rounding::mode RoundMode = rounding::fastest, overflow::mode OverflowMode = overflow::fastest>
struct rounds;

//! @brief Tagging type so that we can nicely initialize fixed objects with raw param to indicate that the representation value should be used "as is".
struct raw_representation_tag {};
//! @brief Undefined value used to specify that "raw" initializing constructor/assignment should be used.
const raw_representation_tag raw = {};

namespace detail {
	// Implementation details, don't look here, fold down this namespace ;)
	// TODO detect presence of 128-bit int type and include it as a valid choice
	template<int WordLength, bool IsValid = (WordLength == 8 || WordLength == 16 || WordLength == 32 || WordLength == 64)> struct word_length_valid;
	template<int WordLength> struct word_length_valid<WordLength, true> {};

	// prototype of float-to-fixed conversion helper, specializations for negative and nonnegative fractional bits must exist
	// TODO parameterize float-to-fixed conversion on rounding::mode too
	template<class R, int FracBits, class F, bool negative = (FracBits < 0)> struct fixed_from_float_impl;
	// nonnegative specialization of float-to-fixed conversion helper
	template<class R, int FracBits, class F> struct fixed_from_float_impl<R, FracBits, F, false> {
		static R convert(F val) {return static_cast<R>(val * (1ull << FracBits) + .5 * signum(val));}
	};

	template<class R, int FracBits, class F> struct fixed_from_float_impl<R, FracBits, F, true> {
		static R convert(F val) {return static_cast<R>(val / (1ull << -FracBits) + .5 * signum(val));}
	};

	template<class R, int FracBits, class F> inline R fixed_from_float(F val) {return fixed_from_float_impl<R, FracBits, F>::convert(val);}

	template<class R, int FracBits, class F, bool negative = (FracBits < 0), bool is_float = (std::numeric_limits<F>::is_specialized && !std::numeric_limits<F>::is_integer)> struct float_from_fixed_impl;
	template<class R, int FracBits, class F> struct float_from_fixed_impl<R, FracBits, F, false, true> {
		static F convert(R val) {return static_cast<F>(val) / (1ull << FracBits);}
	};

	template<class R, int FracBits, class F> struct float_from_fixed_impl<R, FracBits, F, true, true> {
		static F convert(F val) {return static_cast<F>(val) * (1ull << -FracBits);}
	};

	template<class F, int FracBits, class R> inline F float_from_fixed(R val) {
		return float_from_fixed_impl<R, FracBits, F>::convert(val);
	}

	template<class R, int Shift, bool negative = (Shift < 0)> struct shift_right_impl;
	template<class R, int Shift> struct shift_right_impl<R, Shift, false> {static R shift(R val) {return val >> Shift;}	};
	template<class R, int Shift> struct shift_right_impl<R, Shift, true> {static R shift(R val) {return val << Shift;} };
	template<int Shift, class R> inline R shift_right(R val) {return shift_right_impl<R, Shift>::shift(val);}

	template<int V0, int V1> struct max {static const int value = (V0 > V1 ? V0 : V1);};

} // namespace detail

template<int WordLength, int IntBits, bool IsSigned>
struct fixed_properties: private detail::word_length_valid<WordLength>
{
	static const bool is_signed = IsSigned;							//!< Is this signed of unsigned fixed-point type?
	static const int sign_bits = (is_signed ? 1 : 0);				//!< Number of sign bits (0 or 1).
	static const int word_length = WordLength;									//!< Well... word length (in bits).
	static const int integer_bits = IntBits;									//!< Number of integer bits.
	static const int fractional_bits = word_length - integer_bits - sign_bits;	//!< Number of fractional bits.
};


//! @brief Holds fixed-point arithmetic operation result type constants, so that we don't have name clashes and can nicely qualify values like result::max_range.
namespace result {
	//! @brief Fixed point arithmetic operation result type.
	enum type {
		max_range, 		//!< Result word is shifted left, so that integer range is maximized.
		max_precision,	//!< Result word is shifted right, so that fractional precision is maximized.
	};
};

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
struct multiply_result_base;


/*!
 * @brief Implementation of fixed-point number with parameterized word length, number of integer and fractional bits and signedness.
 * @tparam WordLength total number of bits the number is stored in. Must be one of: 8, 16, 32, 64.
 * @tparam IntBits number of integer bits. Number of fractional bits is inferred from @p WordLength, @p IntBits and @p IsSigned.
 * @tparam IsSigned specifies whether this is signed or unsigned number.
 * @pre Only word lengths that can be represented by built-in integer types are allowed. This is enforced by detail::word_length_valid compile-time check.
 */
template<int WordLength, int IntBits, bool IsSigned = true>
class fixed: public fixed_properties<WordLength, IntBits, IsSigned>
{
	typedef typename select_int<WordLength, IsSigned>::type R;
	R v_;
public:
	typedef fixed_properties<WordLength, IntBits, IsSigned> properties;
	using properties::is_signed;
	using properties::sign_bits;
	using properties::word_length;
	using properties::integer_bits;
	using properties::fractional_bits;
	typedef R representation_type;									//!< Type of underlying integer representation.

	//! @brief Default constructor, initializes fixed point number to 0.
	fixed(): v_(R()) {}

	//! @brief Trivial copy constructor from the same type, simply copy the representation value.
	//! @param[in] rhs fixed-point number to initialize this to.
	fixed(const fixed& rhs): v_(rhs.v_) {}

	//! @brief Initialize this fixed-point number with raw integer representation. Useful for interacting with outside-world
	//! (e.g. using data generated by MATLAB).
	//! @param[in] v the value to initialize representation with.
	explicit fixed(R v, const raw_representation_tag&): v_(v) {}

	//! @todo add dsp::rounding::mode param and dsp::overflow::mode param, currently rounding to nearest (like matlab) and wrapping
	explicit fixed(float v): v_(detail::fixed_from_float<R, fractional_bits>(v)) {}
	explicit fixed(double v): v_(detail::fixed_from_float<R, fractional_bits>(v)) {}
	explicit fixed(long double v): v_(detail::fixed_from_float<R, fractional_bits>(v)) {}


	//! @brief Obtain raw integer representation.
	//! @return raw integer representation value.
	representation_type raw() const {return v_;}

	//! @brief Trivial copy constructor from the same type, simply copy the representation value.
	//! @param[in] rhs fixed-point number copy.
	//! @return *this. */
	fixed& operator=(const fixed& rhs) {v_ = rhs.v_; return *this;}

	friend bool operator==(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ == rhs.v_;}
	friend bool operator!=(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ != rhs.v_;}
	friend bool operator<(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ < rhs.v_;}
	friend bool operator<=(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ <= rhs.v_;}
	friend bool operator>(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ > rhs.v_;}
	friend bool operator>=(const fixed<WordLength, IntBits, IsSigned>& lhs, const fixed<WordLength, IntBits, IsSigned>& rhs) {return lhs.v_ > rhs.v_;}


	template<rounding::mode RoundingMode, overflow::mode OverflowMode>
	fixed round(int fractional_bits = 0) {
		return rounds<word_length, integer_bits, is_signed, RoundingMode, OverflowMode>()(*this, fractional_bits);
	}

	template<rounding::mode RoundingMode>
	fixed round(int fractional_bits = 0) {
		return rounds<word_length, integer_bits, is_signed, RoundingMode>()(*this, fractional_bits);
	}

	fixed round(int fractional_bits = 0) {
		return rounds<word_length, integer_bits, is_signed>()(*this, fractional_bits);
	}

private:
};

template<class Float, int WordLength, int IntBits, bool IsSigned>
inline Float float_cast(const fixed<WordLength, IntBits, IsSigned>& f)
{return detail::float_from_fixed<Float, fixed<WordLength, IntBits, IsSigned>::fractional_bits>(f.raw());}


#define DSP_FI_TPARAMS_DECL(index) 	int WordLength ## index, int IntBits ## index, bool IsSigned ## index
#define DSP_FI_TPARAMS(index)		WordLength ## index, IntBits ## index, IsSigned ## index

#define DSP_FI_UNA_TPARAMS_DECL 	DSP_FI_TPARAMS_DECL(0)
#define DSP_FI_UNA_TPARAMS 			DSP_FI_TPARAMS(0)

#define DSP_FI_BIN_TPARAMS_DECL 	DSP_FI_TPARAMS_DECL(0), DSP_FI_TPARAMS_DECL(1)
#define DSP_FI_BIN_TPARAMS 			DSP_FI_TPARAMS(0), DSP_FI_TPARAMS(1)

template<int WordLength, int IntBits, bool IsSigned, rounding::mode RoundMode, overflow::mode OverflowMode>
struct rounds: public std::binary_function<fixed<WordLength, IntBits, IsSigned>, int, fixed<WordLength, IntBits, IsSigned> >
{
	fixed<WordLength, IntBits, IsSigned> operator()(const fixed<WordLength, IntBits, IsSigned>& f, int fractional_bits) {
		int at_bit = fixed<WordLength, IntBits, IsSigned>::fractional_bits - fractional_bits;
		if (at_bit <= 0)
			return f;

		typedef typename fixed<WordLength, IntBits, IsSigned>::representation_type R;
		R val = dsp::round<RoundMode, OverflowMode>(f.raw(), at_bit);
		return fixed<WordLength, IntBits, IsSigned>(val, raw);
	}
};

//template<int WordLengthArg, int IntBitsArg, bool IsSignedArg, int WordLengthRes, int IntBitsRes = IntBitsArg, bool IsSignedRes = IsSignedArg, overflow::mode OverflowMode = overflow::wrap, rounding::mode RoundMode = rounding::fastest>
//struct converts;


template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
struct multiply_result_base
{
	//! @brief Multiply result is signed if any of the operands is signed.
	static const bool is_signed = (IsSigned0 || IsSigned1);
	//! @brief Number of sign bits (0 or 1)
	static const int sign_bits = (is_signed ? 1 : 0);
	//! @brief Choose word length of largest operand as base word length.
	static const int base_word_length = (WordLength0 > WordLength1 ? WordLength0 : WordLength1);
	//! @brief 'Definition' word length of full-precision result is double the base length (actually WordLength0 + WordLength1 but we quantize it to the nearest higher power-of-2).
	static const int word_length = 2 * base_word_length;
	//! @brief Number of fractional bits in first operand.
	static const int fractional_bits_0 = fixed_properties<WordLength0, IntBits0, IsSigned0>::fractional_bits;
	//! @brief Number of fractional bits in second operand.
	static const int fractional_bits_1 = fixed_properties<WordLength1, IntBits1, IsSigned1>::fractional_bits;
	//! @brief Minimum required number of fractional bits in the result.
	static const int min_fractional_bits = fractional_bits_0 + fractional_bits_1;
	//! @brief Minimum required number of integer bits in the result.
	static const int min_integer_bits = IntBits0 + IntBits1 + (IsSigned0 ? 1 : 0) + (IsSigned1 ? 1 : 0) - sign_bits;
	//! @brief Representation type of full-precision result
	typedef typename select_int<word_length, is_signed>::type representation_type;
	//! @brief Type of result when truncated to base word length.
	typedef fixed<base_word_length, min_integer_bits, is_signed> type_base;
	//! @brief Type of first operand
	typedef fixed<WordLength0, IntBits0, IsSigned0> operand_type_0;
	//! @brief Type of second operand
	typedef fixed<WordLength1, IntBits1, IsSigned1> operand_type_1;
};

//! @brief 'Traits' type describing the properties of the result of multiplication of 2 fixed-point values.
template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1, result::type ResultType = result::max_range>
struct multiply_result;

template<DSP_FI_BIN_TPARAMS_DECL>
struct multiply_result<DSP_FI_BIN_TPARAMS, result::max_range>: public multiply_result_base<DSP_FI_BIN_TPARAMS>
{
	typedef multiply_result_base<DSP_FI_BIN_TPARAMS> base;
	//! @brief Number of integer bits in max-range (right-shifted) result.
	static const int integer_bits = base::word_length - base::min_fractional_bits - base::sign_bits;
	//! @brief Number of fractional bits.
	static const int fractional_bits = base::min_fractional_bits;
	//! @brief Type of 'max-range' (right-shifted) result.
	typedef fixed<base::word_length, integer_bits, base::is_signed> type;
};

template<DSP_FI_BIN_TPARAMS_DECL>
struct multiply_result<DSP_FI_BIN_TPARAMS, result::max_precision>: public multiply_result_base<DSP_FI_BIN_TPARAMS>
{
	typedef multiply_result_base<DSP_FI_BIN_TPARAMS> base;
	//! @brief Number of integer bits.
	static const int integer_bits = base::min_integer_bits;
	//! @brief Number of fractional bits in 'max-precision' (left-shifted) result.
	static const int fractional_bits = base::word_length - base::min_integer_bits - base::sign_bits;
	//! @brief Type of 'max-precision' (left-shifted) result.
	typedef fixed<base::word_length, integer_bits, base::is_signed> type;
};

//! @brief Multiplication functor parameterized with result::type constants, returning full-precision (double-size) result.
template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1, result::type ResultType = result::max_range>
struct multiplies_lossless;

template<DSP_FI_BIN_TPARAMS_DECL>
struct multiplies_lossless<DSP_FI_BIN_TPARAMS, result::max_range>:
	public std::binary_function< fixed<DSP_FI_TPARAMS(0)>, fixed<DSP_FI_TPARAMS(1)>, typename multiply_result<DSP_FI_BIN_TPARAMS, result::max_range>::type >
{
	typedef multiply_result<DSP_FI_BIN_TPARAMS, result::max_range> result_traits;
	typename result_traits::type operator()(const fixed<DSP_FI_TPARAMS(0)>& l, const fixed<DSP_FI_TPARAMS(1)>& r) {
		typedef typename result_traits::type Res; // this is the full-precision fixed type
		typedef typename Res::representation_type R; // this is the int type
		return Res(static_cast<R>(l.raw()) * static_cast<R>(r.raw()), raw); // simply convert to target int type and do copy, no philosophy here
	}
};

template<DSP_FI_BIN_TPARAMS_DECL>
struct multiplies_lossless<DSP_FI_BIN_TPARAMS, result::max_precision>:
	public std::binary_function< fixed<DSP_FI_TPARAMS(0)>, fixed<DSP_FI_TPARAMS(1)>, typename multiply_result<DSP_FI_BIN_TPARAMS, result::max_precision>::type >
{
	typedef multiply_result<DSP_FI_BIN_TPARAMS, result::max_precision> result_traits;
	typename result_traits::type operator()(const fixed<DSP_FI_TPARAMS(0)>& l, const fixed<DSP_FI_TPARAMS(1)>& r) {
		typedef typename result_traits::type Res; // this is the full-precision fixed type
		typedef typename Res::representation_type R; // this is the int type
		R val = static_cast<R>(l.raw()) * static_cast<R>(r.raw()); // do the multiplication, the result is now in Qmin_integer_bits.min_fractional_bits format
		val <<= (result_traits::fractional_bits - result_traits::min_fractional_bits); // shift result left to maximize bit width of fractional part
		return Res(val, raw); // simply convert to target int type and do copy, no philosophy here
	}
};

template<result::type ResultType, int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline typename multiply_result<DSP_FI_BIN_TPARAMS, ResultType>::type
multiply_lossless(const fixed<DSP_FI_TPARAMS(0)>& lhs, const fixed<DSP_FI_TPARAMS(1)>& rhs) {
	return multiplies_lossless<DSP_FI_BIN_TPARAMS, ResultType>()(lhs, rhs);
}

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1,
		int WordLengthR,	//!< Word length of the result.
		int IntBitsR,		//!< Integral bits of the result.
		bool IsSignedR = (IsSigned0 || IsSigned1), //!< Signedness of the result
		rounding::mode RoundMode = rounding::fastest, overflow::mode OverflowMode = overflow::fastest>
struct multiplies: public std::binary_function<fixed<WordLength0, IntBits0, IsSigned0>,
	fixed<WordLength1, IntBits1, IsSigned1>, fixed<WordLengthR, IntBitsR, IsSignedR> >
{
	typedef fixed<WordLengthR, IntBitsR, IsSignedR> result_type;
	result_type operator()(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs) {
		typedef multiply_result_base<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1> T;
		typedef typename T::representation_type R;
		R res = static_cast<R>(lhs.raw()) * static_cast<R>(rhs.raw());
		if (T::min_fractional_bits > result_type::fractional_bits) {
			res = dsp::round<RoundMode, OverflowMode>(res, T::min_fractional_bits - result_type::fractional_bits);
		}
		else {
			// TODO check overflow
		}
		res = detail::shift_right<T::min_fractional_bits - result_type::fractional_bits>(res);
		return result_type(static_cast<R>(res), raw);
	}
};

template<int WordLengthR, int IntBitsR, rounding::mode RoundMode, overflow::mode OverflowMode,
	int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<WordLengthR, IntBitsR, (IsSigned0 || IsSigned1)>
multiply(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs)
{
	return multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, WordLengthR, IntBitsR, (IsSigned0 || IsSigned1), RoundMode, OverflowMode>()(lhs, rhs);
}

template<int WordLengthR, int IntBitsR, rounding::mode RoundMode,
	int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<WordLengthR, IntBitsR, (IsSigned0 || IsSigned1)>
multiply(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs)
{
	return multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, WordLengthR, IntBitsR, (IsSigned0 || IsSigned1), RoundMode>()(lhs, rhs);
}

template<int WordLengthR, int IntBitsR, overflow::mode OverflowMode,
	int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<WordLengthR, IntBitsR, (IsSigned0 || IsSigned1)>
multiply(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs)
{
	return multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, WordLengthR, IntBitsR, (IsSigned0 || IsSigned1), rounding::fastest, OverflowMode>()(lhs, rhs);
}

template<int WordLengthR, int IntBitsR,
	int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<WordLengthR, IntBitsR, (IsSigned0 || IsSigned1)>
multiply(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs)
{
	return multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, WordLengthR, IntBitsR, (IsSigned0 || IsSigned1)>()(lhs, rhs);
}

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<detail::max<WordLength0,WordLength1>::value, detail::max<IntBits0,IntBits1>::value, (IsSigned0 || IsSigned1)>
operator* (const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs)
{
	return multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, detail::max<WordLength0,WordLength1>::value, detail::max<IntBits0,IntBits1>::value, (IsSigned0 || IsSigned1)>()(lhs, rhs);
}

} } // namespace dsp::fi

namespace std { // specializing std::numeric_limits for dsp::fi::fixed

template<int WordLength, int IntBits, bool IsSigned>
class numeric_limits<dsp::fi::fixed<WordLength, IntBits, IsSigned> >
{
	typedef dsp::fi::fixed<WordLength, IntBits, IsSigned> F;
	typedef typename F::representation_type R;
	typedef std::numeric_limits<R> NL;
public:

    static const bool is_specialized = true;

    static F min() throw() {return F(1, dsp::fi::raw);}
    static F max() throw() {return F(NL::max(), dsp::fi::raw);}
    static F lowest() throw() {return F(NL::min(), dsp::fi::raw);}

    static const int digits = IntBits;
    static const int digits10 = (int)(digits * 301L / 1000);
    static const bool is_signed = IsSigned;
    static const bool is_integer = false;
    static const bool is_exact = true;
    static const int radix = 2;

    static F epsilon() throw() {return F(1, dsp::fi::raw);}
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
