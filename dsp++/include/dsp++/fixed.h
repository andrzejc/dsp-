/*!
 * @file dsp++/fixed.h
 * @brief Fixed-point number template and associated stuff.
 */
#ifndef DSP_FIXED_H_INCLUDED
#define DSP_FIXED_H_INCLUDED
#pragma once

#include <dsp++/stdint.h>

#include <limits>
#include <functional>

namespace dsp { namespace fi {

//! @brief Holds fixed-point (and other) rounding mode constants, so that we don't have name clashes and can nicely qualify values like round::fastest.
namespace round {
	//! @brief Rounding mode.
	enum mode {
		fastest = std::round_indeterminate,			//!< Speed is more important than the choice in value.
		negative = std::round_toward_neg_infinity,	//!< Round towards negative infinity. This mode is useful in interval arithmetic.
		truncated = std::round_toward_zero,			//!< Round towards zero. This mode is useful in implementing integral arithmetic.
		positive = std::round_toward_infinity,		//!< Round towards positive infinity. This mode is useful in interval arithmetic.
		nearest = std::round_to_nearest,			//!< Round towards the nearest value, but exactly-half values are rounded towards maximum magnitude. This mode is the standard school algorithm.
		near_even, 			//!< Round towards the nearest value, but exactly-half values are rounded towards even values. This mode has more balance than the classic mode.
		near_odd,			//!< Round towards the nearest value, but exactly-half values are rounded towards odd values. This mode has as much balance as the near_even mode, but preserves more information.
	};
} // namespace round

template<int WordLength, int IntBits, bool IsSigned, round::mode RoundMode = round::fastest>
struct rounds;

//! @brief Holds fixed-point (and other) overflow mode constants, so that we don't have name clashes and can nicely qualify values like overflow::saturate.
namespace overflow {
	//! @brief Overflow mode.
	enum mode {
		wrap,			//!< Overflowed values will wrap.
		saturate,		//!< If the dynamic value exceeds the range of the variable, assign the nearest representable value.
		exception,		//!< If the dynamic value exceeds the range of the variable, throw an exception of type std::overflow_error.
	};
} // namespace overflow

//! @brief Tagging type so that we can nicely initialize fixed objects with raw param to indicate that the representation value should be used "as is".
struct raw_representation_tag {};
//! @brief Undefined value used to specify that "raw" initializing constructor/assignment should be used.
const raw_representation_tag raw = {};

namespace detail {
	// Implementation details, don't look here (fold down this namespace) ;)
	template<int WordLength, bool IsValid = (WordLength == 8 || WordLength == 16 || WordLength == 32 || WordLength == 64)> struct word_length_valid;
	template<int WordLength> struct word_length_valid<WordLength, true> {};

	template <typename T> int sgn(T val) {
	   return (T(0) < val) - (val < T(0));
	}

	template<class R, int FracBits, class F, bool negative = (FracBits < 0)> struct fixed_from_float_impl;
	template<class R, int FracBits, class F> struct fixed_from_float_impl<R, FracBits, F, false> {
		static R convert(F val) {
			return static_cast<R>(val * (1ull << FracBits) + .5 * sgn(val));
		}
	};

	template<class R, int FracBits, class F> struct fixed_from_float_impl<R, FracBits, F, true> {
		static R convert(F val) {
			return static_cast<R>(val / (1ull << -FracBits) + .5 * sgn(val));
		}
	};

	template<class R, int FracBits, class F> R fixed_from_float(F val) {
		return fixed_from_float_impl<R, FracBits, F>::convert(val);
	}

	template<class R, int FracBits, class F, bool negative = (FracBits < 0), bool is_float = !std::numeric_limits<F>::is_integer> struct float_from_fixed_impl;
	template<class R, int FracBits, class F> struct float_from_fixed_impl<R, FracBits, F, false, true> {
		static F convert(R val) {
			return static_cast<F>(val) / (1ull << FracBits);
		}
	};

	template<class R, int FracBits, class F> struct float_from_fixed_impl<R, FracBits, F, true, true> {
		static F convert(F val) {
			return static_cast<F>(val) * (1ull << -FracBits);
		}
	};

	template<class F, int FracBits, class R> F float_from_fixed(R val) {
		return float_from_fixed_impl<R, FracBits, F>::convert(val);
	}

	template<class R, round::mode Mode, bool IsSigned = std::numeric_limits<R>::is_signed> struct round_impl;

	// discard all the bits below the specified one
	template<class R, bool IsSigned> struct round_impl<R, round::truncated, IsSigned> {
		static R round(R val, int at_bit) {
			R div = R(1) << at_bit;
			R r = val % div;
			val -= r;
			return val;
		}
	};

	template<class R, bool IsSigned> struct round_impl<R, round::nearest, IsSigned> {
		static R round(R val, int at_bit) {
			R div = R(1) << at_bit;
			R r = val % div;
			if (0 == r)
				return val;
			val -= r;
			using std::abs;
			if (std::abs(r) >= div / 2)
				val += sgn(r) * div;
			return val;
		}
	};

	template<class R, bool IsSigned> struct round_impl<R, round::positive, IsSigned> {
		static R round(R val, int at_bit) {
			R div = R(1) << at_bit;
			R r = val % div;
			if (0 == r)
				return val;
			val -= r;
			if (r > 0)
				val += div;
			return val;
		}
	};

	template<class R, bool IsSigned> struct round_impl<R, round::negative, IsSigned> {
		static R round(R val, int at_bit) {
			R div = R(1) << at_bit;
			R r = val % div;
			if (0 == r)
				return val;
			val -= r;
			if (!(r > 0))
				val -= div;
			return val;
		}
	};


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

	//! @todo add dsp::fi::round::mode param and dsp::fi::overflow::mode param, currently rounding to nearest (like matlab) and wrapping
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


	template<round::mode RoundingMode>
	fixed round(int fractional_bits = 0) {
		return rounds<word_length, integer_bits, is_signed, RoundingMode>()(*this, fractional_bits);
	}

	template<class Float>
	friend Float float_cast(fixed f) {return detail::float_from_fixed<Float, fractional_bits>(f.v_);}
private:
};

#define DSP_FI_TPARAMS_DECL(index) 	int WordLength ## index, int IntBits ## index, bool IsSigned ## index
#define DSP_FI_TPARAMS(index)		WordLength ## index, IntBits ## index, IsSigned ## index

#define DSP_FI_UNA_TPARAMS_DECL 	DSP_FI_TPARAMS_DECL(0)
#define DSP_FI_UNA_TPARAMS 			DSP_FI_TPARAMS(0)

#define DSP_FI_BIN_TPARAMS_DECL 	DSP_FI_TPARAMS_DECL(0), DSP_FI_TPARAMS_DECL(1)
#define DSP_FI_BIN_TPARAMS 			DSP_FI_TPARAMS(0), DSP_FI_TPARAMS(1)

template<int WordLength, int IntBits, bool IsSigned, round::mode RoundMode>
struct rounds: public std::binary_function<fixed<WordLength, IntBits, IsSigned>, int, fixed<WordLength, IntBits, IsSigned> >
{
	fixed<WordLength, IntBits, IsSigned> operator()(fixed<WordLength, IntBits, IsSigned> f, int fractional_bits) {
		int at_bit = fixed<WordLength, IntBits, IsSigned>::fractional_bits - fractional_bits;
		if (at_bit <= 0)
			return f;

		typedef typename fixed<WordLength, IntBits, IsSigned>::representation_type R;
		R val = detail::round_impl<R, RoundMode>::round(f.raw(), at_bit);
		return fixed<WordLength, IntBits, IsSigned>(val, raw);
	}
};

//template<int WordLengthArg, int IntBitsArg, bool IsSignedArg, int WordLengthRes, int IntBitsRes = IntBitsArg, bool IsSignedRes = IsSignedArg, overflow::mode OverflowMode = overflow::wrap, round::mode RoundMode = round::fastest>
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
	static const int min_integer_bits = IntBits0 + IntBits1;
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
	typename result_traits::type operator()(fixed<DSP_FI_TPARAMS(0)> l, fixed<DSP_FI_TPARAMS(1)> r) {
		typedef typename result_traits::type Res; // this is the full-precision fixed type
		typedef Res::representation_type R; // this is the int type
		return Res(static_cast<R>(l.raw()) * static_cast<R>(r.raw()), raw); // simply convert to target int type and do copy, no philosophy here
	}
};

template<DSP_FI_BIN_TPARAMS_DECL>
struct multiplies_lossless<DSP_FI_BIN_TPARAMS, result::max_precision>:
	public std::binary_function< fixed<DSP_FI_TPARAMS(0)>, fixed<DSP_FI_TPARAMS(1)>, typename multiply_result<DSP_FI_BIN_TPARAMS, result::max_precision>::type >
{
	typedef multiply_result<DSP_FI_BIN_TPARAMS, result::max_precision> result_traits;
	typename result_traits::type operator()(fixed<DSP_FI_TPARAMS(0)> l, fixed<DSP_FI_TPARAMS(1)> r) {
		typedef typename result_traits::type Res; // this is the full-precision fixed type
		typedef Res::representation_type R; // this is the int type
		R val = static_cast<R>(l.raw()) * static_cast<R>(r.raw()); // do the multiplication
		val <<= (Res::fractional_bits - Res::min_fractional_bits); // shift result left to maximize bit width of fractional part
		return Res(val, raw); // simply convert to target int type and do copy, no philosophy here
	}
};

template<result::type ResultType, int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1>
typename multiply_result<DSP_FI_BIN_TPARAMS, ResultType>::type
multiply_lossless(fixed<DSP_FI_TPARAMS(0)> lhs, fixed<DSP_FI_TPARAMS(1)> rhs) {
	return multiplies_lossless<DSP_FI_BIN_TPARAMS, ResultType>()(lhs, rhs);
}

////! @brief Multiplication functor parameterized returning base-type-sized result, scaled as appropriate
//template<int WordLength0, int IntBits0, bool IsSigned0, int WordLength1, int IntBits1, bool IsSigned1, int Scale = 0>
//struct multiplies_scaled


} }  // namespace dsp::fi

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
