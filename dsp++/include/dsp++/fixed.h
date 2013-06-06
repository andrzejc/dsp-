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

//! @brief Fixed-point conversion function object. Converts the argument passed to operator() to other fixed-point type
//! performing all the necessary rounding and overflow checking according to the template params.
template<int WordLength, int IntBits, bool IsSigned, int WordLengthR, int IntBitsR, bool IsSignedR = IsSigned,
		rounding::mode RoundingMode = rounding::fastest, overflow::mode OverflowMode = overflow::fastest>
struct converts;

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
	template<class R, int Shift> struct shift_right_impl<R, Shift, true> {static R shift(R val) {return val << -Shift;} };
	template<int Shift, class R> inline R shift_right(R val) {return shift_right_impl<R, Shift>::shift(val);}

	template<class R, int Shift, bool negative = (Shift < 0)> struct shift_left_impl;
	template<class R, int Shift> struct shift_left_impl<R, Shift, false> {static R shift(R val) {return val << Shift;}	};
	template<class R, int Shift> struct shift_left_impl<R, Shift, true> {static R shift(R val) {return val >> -Shift;} };
	template<int Shift, class R> inline R shift_left(R val) {return shift_left_impl<R, Shift>::shift(val);}

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


template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
struct multiply_result;


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
	fixed round(int fractional_bits = 0) const
	{return rounds<word_length, integer_bits, is_signed, RoundingMode, OverflowMode>()(*this, fractional_bits);}

	template<rounding::mode RoundingMode>
	fixed round(int fractional_bits = 0) const
	{return rounds<word_length, integer_bits, is_signed, RoundingMode>()(*this, fractional_bits);}

	fixed round(int fractional_bits = 0) const
	{return rounds<word_length, integer_bits, is_signed>()(*this, fractional_bits);}

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

template<int WordLength, int IntBits, bool IsSigned, int WordLengthR, int IntBitsR, bool IsSignedR,
		rounding::mode RoundingMode, overflow::mode OverflowMode>
struct converts: public std::unary_function<fixed<WordLength, IntBits, IsSigned>, fixed<WordLengthR, IntBitsR, IsSignedR> >
{
	fixed<WordLengthR, IntBitsR, IsSignedR> operator()(const fixed<WordLength, IntBits, IsSigned>& lhs) {
		typedef fixed<WordLengthR, IntBitsR, IsSignedR> Target;
		typedef fixed<WordLength, IntBits, IsSigned> Source;
		typedef typename Source::representation_type R;
		typedef typename select_int<detail::max<WordLength, WordLengthR>::value, IsSignedR>::type IR; // intermediate rep type
		IR res = static_cast<IR>(lhs.raw());
		if (Source::is_signed && !Target::is_signed && lhs.raw() < R())
			// when converting negative value to unsigned type start with overflow handling, so that we get exception or set result to 0 early
			dsp::overflow_handle<OverflowMode>(res, false);
		if (Source::fractional_bits > Target::fractional_bits) { // we will need to shift the result to the right by the difference
			IR adj = dsp::rounding_adjustment<RoundingMode>(res, Source::fractional_bits - Target::fractional_bits); 	// find the rounding adjustment, but don't round yet to avoid possible overflow
			const IR scale = (IR(1)) << (static_cast<IR>(Source::fractional_bits - Target::fractional_bits));			// the scaling factor, spurious parentheses to make MSVC shut up
			res = dsp::add<OverflowMode>(res / scale, adj / scale);														// now scale both result and adjustment, and add them watching for overflow
		}
		else
			res = detail::shift_left<Target::fractional_bits - Source::fractional_bits>(res);
		dsp::overflow_check_handle<OverflowMode>(res, Target::fractional_bits + Target::integer_bits);	 // check if the result will overflow
		return Target(static_cast<typename Target::representation_type>(res), dsp::fi::raw);
	}
};

template<class FixedR, rounding::mode RoundingMode, overflow::mode OverflowMode, int WordLength, int IntBits, bool IsSigned>
inline FixedR fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, FixedR::word_length, FixedR::integer_bits, FixedR::is_signed, RoundingMode, OverflowMode>()(lhs);
}

template<class FixedR, rounding::mode RoundingMode, int WordLength, int IntBits, bool IsSigned>
inline FixedR fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, FixedR::word_length, FixedR::integer_bits, FixedR::is_signed, RoundingMode>()(lhs);
}

template<class FixedR, int WordLength, int IntBits, bool IsSigned>
inline FixedR fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, FixedR::word_length, FixedR::integer_bits, FixedR::is_signed>()(lhs);
}

template<int WordLengthR, int IntBitsR, bool IsSignedR, rounding::mode RoundingMode, overflow::mode OverflowMode, int WordLength, int IntBits, bool IsSigned>
inline fixed<WordLengthR, IntBitsR, IsSignedR> fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, WordLengthR, IntBitsR, IsSignedR, RoundingMode, OverflowMode>()(lhs);
}

template<int WordLengthR, int IntBitsR, bool IsSignedR, rounding::mode RoundingMode, int WordLength, int IntBits, bool IsSigned>
inline fixed<WordLengthR, IntBitsR, IsSignedR> fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, WordLengthR, IntBitsR, IsSignedR, RoundingMode>()(lhs);
}

template<int WordLengthR, int IntBitsR, bool IsSignedR, int WordLength, int IntBits, bool IsSigned>
inline fixed<WordLengthR, IntBitsR, IsSignedR> fixed_cast(const fixed<WordLength, IntBits, IsSigned>& lhs) {
	return converts<WordLength, IntBits, IsSigned, WordLengthR, IntBitsR, IsSignedR>()(lhs);
}

//! @brief 'Traits' type describing the properties of the result of multiplication of 2 fixed-point values.
template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
struct multiply_result
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
	//! @brief Number of integer bits taking into account that actual word length may be wider than sum of operands' word lengths.
	static const int integer_bits = word_length - min_fractional_bits - sign_bits;
	//! @brief Number of fractional bits.
	static const int fractional_bits = min_fractional_bits;
	//! @brief Type of 'max-range' (right-shifted) result.
	typedef fixed<word_length, integer_bits, is_signed> type;
};

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
struct multiplies_lossless: public std::binary_function<fixed<DSP_FI_TPARAMS(0)>, fixed<DSP_FI_TPARAMS(1)>,
	typename multiply_result<DSP_FI_BIN_TPARAMS>::type>
{
	typedef multiply_result<DSP_FI_BIN_TPARAMS> result_traits;
	typename result_traits::type operator()(const fixed<DSP_FI_TPARAMS(0)>& l, const fixed<DSP_FI_TPARAMS(1)>& r) {
		typedef typename result_traits::type Res; // this is the full-precision fixed type
		typedef typename Res::representation_type R; // this is the int type
		return Res(static_cast<R>(l.raw()) * static_cast<R>(r.raw()), raw); // simply convert to target int type and multiply, no rounding/overflow can happen here
	}
};

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline typename multiply_result<DSP_FI_BIN_TPARAMS>::type
multiply_lossless(const fixed<DSP_FI_TPARAMS(0)>& lhs, const fixed<DSP_FI_TPARAMS(1)>& rhs) {
	return multiplies_lossless<DSP_FI_BIN_TPARAMS>()(lhs, rhs);
}

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1,
		int WordLengthR,	//!< Word length of the result.
		int IntBitsR,		//!< Integral bits of the result.
		bool IsSignedR = (IsSigned0 || IsSigned1), //!< Signedness of the result
		rounding::mode RoundMode = rounding::fastest, overflow::mode OverflowMode = overflow::fastest>
struct multiplies: public std::binary_function<fixed<WordLength0, IntBits0, IsSigned0>,
	fixed<WordLength1, IntBits1, IsSigned1>, fixed<WordLengthR, IntBitsR, IsSignedR> >
{
	fixed<WordLengthR, IntBitsR, IsSignedR> operator()(const fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs) {
		return fixed_cast<fixed<WordLengthR, IntBitsR, IsSignedR>, RoundMode, OverflowMode>(multiply_lossless(lhs, rhs));
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

template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
inline fixed<WordLength0, IntBits0, IsSigned0>&
operator*=(fixed<WordLength0, IntBits0, IsSigned0>& lhs, const fixed<WordLength1, IntBits1, IsSigned1>& rhs) {
	return lhs = multiplies<WordLength0, IntBits0, IsSigned0, WordLength1, IntBits1, IsSigned1, WordLength0, IntBits0, IsSigned0>()(lhs, rhs);
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
