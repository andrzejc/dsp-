/*!
 * @file dsp++/intmath.h
 * @brief Integer (and not only) maths helpers. Overflow checking, rounding, basic operations.
 * @author Andrzej
 */
#ifndef DSP_INTMATH_H_INCLUDED
#define DSP_INTMATH_H_INCLUDED
#pragma once

#include <limits>
#include <stdexcept>
#include <cmath>

namespace dsp {

//! @brief Holds fixed-point (and other) rounding mode constants, so that we don't have name clashes and can nicely qualify values like rounding::fastest.
namespace rounding {
	//! @brief Rounding mode.
	enum mode {
		fastest,				//!< Speed is more important than the choice in value.
		truncated = fastest,	//!< Round towards zero. This mode is useful in implementing integral arithmetic.
		negative,				//!< Round towards negative infinity. This mode is useful in interval arithmetic.
		positive,				//!< Round towards positive infinity. This mode is useful in interval arithmetic.
		nearest,				//!< Round towards the nearest value, but exactly-half values are rounded towards maximum magnitude. This mode is the standard school algorithm.
//		near_even, 				//!< Round towards the nearest value, but exactly-half values are rounded towards even values. This mode has more balance than the classic mode.
//		near_odd,				//!< Round towards the nearest value, but exactly-half values are rounded towards odd values. This mode has as much balance as the near_even mode, but preserves more information.
	};
} // namespace rounding

//! @brief Holds fixed-point (and other) overflow mode constants, so that we don't have name clashes and can nicely qualify values like overflow::saturate.
namespace overflow {
	//! @brief Overflow mode.
	enum mode {
		fastest,		//!< Implementation-defined overflow mode (wrap or saturate, if hardware supports saturated maths).
		wrap = fastest,			//!< Overflowed values will wrap.
		saturate,		//!< If the dynamic value exceeds the range of the variable, assign the nearest representable value.
		exception,		//!< If the dynamic value exceeds the range of the variable, throw an exception of type std::overflow_error.
	};
} // namespace overflow

namespace detail { // implementation details, not really interesting
// signum function implementation
template<class R, bool is_signed = std::numeric_limits<R>::is_signed>
struct signum_impl; // leave default unimplemented

template<class R> struct signum_impl<R, false> // unsigned specialization - so that we don't get warnings when testing if unsigned values are less than 0
{static R signum(R val) {return static_cast<R>(val > R());}};

template<class R> struct signum_impl<R, true> // signed specialization
{static R signum(R val) {return static_cast<R>(R() < val) - (val < R());}};

} // namespace detail

//! @brief Signum (sign) function.
//! @tparam Type of parameter being tested.
//! @param[in] val value to test.
//! @note For unsigned values the only possible results are 0 and 1, therefore the return value maintains type of
//! the input param so that no additional promotions will be made when the result is used in maths.
//! @return sign of the value (-1, 0, or 1).
template<class R>
inline R signum(R val) {return detail::signum_impl<R>::signum(val);}

namespace detail {
// overflow handlers, by default silently ignore, this is implementation of overflow::fastest (wrap) mode
template<class R, overflow::mode OverflowMode>
struct overflow_handle_impl {static void handle(R&, bool) {}};

template<class R> struct overflow_handle_impl<R, overflow::saturate>
{static void handle(R& val, bool up) {val = (up ? std::numeric_limits<R>::max() : std::numeric_limits<R>::min());}};

template<class R> struct overflow_handle_impl<R, overflow::exception>
{static void handle(R&, bool) {throw std::overflow_error("overflow");}};

} // namespace detail

//! @brief Handle overflow detected by higher-level algorithm.
//! @tparam OverflowMode handling mode
//! @tparam R type of variable in which where overflow occurred.
//! @param[in,out] val overflowed value, may be adjusted upon return.
//! @param[in] up specifies whether overflow occurred in higher or lower direction (overflow/underflow).
template<overflow::mode OverflowMode, class R>
inline void overflow_handle(R& val, bool up) {detail::overflow_handle_impl<R, OverflowMode>::handle(val, up);}

namespace detail {
// overflow-checking add/subtract operations
template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct add_impl;
template<class R> struct add_impl<R, overflow::wrap, true> {static R add(R v0, R v1) {return v0 + v1;}};  // specializations for the trivial (wrapping) case
template<class R> struct add_impl<R, overflow::wrap, false> {static R add(R v0, R v1) {return v0 + v1;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct add_impl<R, OverflowMode, false> {
	static R add(R v0, R v1) {
		bool ovf = (v1 > std::numeric_limits<R>::max() - v0);
		v0 += v1;
		if (ovf)
			overflow_handle<OverflowMode>(v0, true);
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct add_impl<R, OverflowMode, true> {
	static R add(R v0, R v1) {
		bool ovf = false;
		bool up;
		if (v0 < R() && v1 < R()) {
			ovf = (v1 < std::numeric_limits<R>::min() - v0);
			up = false;
		}
		else if (v0 > R() && v1 > R()) {
			ovf = (v1 > std::numeric_limits<R>::max() - v0);
			up = true;
		}
		v0 += v1;
		if (ovf)
			overflow_handle<OverflowMode>(v0, up);
		return v0;
	}
};

template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct sub_impl;
template<class R> struct sub_impl<R, overflow::wrap, true> {static R sub(R v0, R v1) {return v0 - v1;}};  // specializations for the trivial (wrapping) case
template<class R> struct sub_impl<R, overflow::wrap, false> {static R sub(R v0, R v1) {return v0 - v1;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct sub_impl<R, OverflowMode, false> {
	static R sub(R v0, R v1) {
		bool ovf = (v1 > v0);
		v0 -= v1;
		if (ovf)
			overflow_handle<OverflowMode>(v0, false);
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct sub_impl<R, OverflowMode, true> {
	static R sub(R v0, R v1) {
		bool ovf = false;
		bool up;
		if (v0 < R() && v1 > R()) {
			ovf = (v1 > v0 - std::numeric_limits<R>::min());
			up = false;
		}
		else if (v0 > R() && v1 < R()) {
			ovf = (v1 < v0 - std::numeric_limits<R>::max());
			up = true;
		}
		v0 -= v1;
		if (ovf)
			overflow_handle<OverflowMode>(v0, up);
		return v0;
	}
};

template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct mul_impl;
template<class R> struct mul_impl<R, overflow::wrap, true> {static R mul(R v0, R v1) {return v0 * v1;}};  // specializations for the trivial (wrapping) case
template<class R> struct mul_impl<R, overflow::wrap, false> {static R mul(R v0, R v1) {return v0 * v1;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct mul_impl<R, OverflowMode, false> {
	static R mul(R v0, R v1) {
		bool ovf = (v0 != R() && v1 != R() && v0 > std::numeric_limits<R>::max() / v1);
		if (ovf)
			overflow_handle<OverflowMode>(v0, true);
		else
			v0 *= v1;
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct mul_impl<R, OverflowMode, true> {
	static R mul(R v0, R v1) {
		bool ovf = false;
		bool up;
		if (v0 > R()) {
			if (v1 > R()) {
				if (v0 > std::numeric_limits<R>::max() / v1) {
					ovf = true;
					up = true;
				}
			}
			else {
				if (v1 < std::numeric_limits<R>::min() / v0) {
					ovf = true;
					up = false;
				}
			}
		}
		else {
			if (v1 > R()) {
				if (v0 < std::numeric_limits<R>::min() / v1) {
					ovf = true;
					up = false;
				}
			}
			else {
				if ((v0 != R()) && (v1 < std::numeric_limits<R>::max() / v0)) {
					ovf = true;
					up = true;
				}
			}
		}
		if (ovf)
			overflow_handle<OverflowMode>(v0, up);
		else
			v0 *= v1;
		return v0;
	}
};

template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct div_impl;
template<class R> struct div_impl<R, overflow::wrap, true> {static R div(R v0, R v1) {return v0 / v1;}};  // specializations for the trivial (wrapping) case
template<class R> struct div_impl<R, overflow::wrap, false> {static R div(R v0, R v1) {return v0 / v1;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct div_impl<R, OverflowMode, false> {
	static R div(R v0, R v1) {
		bool ovf = (R() == v1 && R() != v0);	// unsigned overflow may happen only when dividing by 0, result is +inf
		if (ovf) 
			overflow_handle<OverflowMode>(v0, false);
		else
			v0 /= v1;
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct div_impl<R, OverflowMode, true> {
	static R div(R v0, R v1) {
		bool ovf = false;
		bool up;
		if (R() == v1 && R() != v0) {
			ovf = true;
			up = (v0 >= R());
		}
		else if (v0 == std::numeric_limits<R>::min() && R(-1) == v1) { // igned overflow may happen also when dividing min by -1, the result is max + 1
			ovf = true;
			up = true;
		}
		if (ovf)
			overflow_handle<OverflowMode>(v0, up);
		else
			v0 /= v1;
		return v0;
	}
};

template<class R, bool IsSigned = std::numeric_limits<R>::is_signed> struct mod_impl;
template<class R> struct mod_impl<R, false> {static R mod(R v0, R v1) {return v0 % v1;}};  // unsigned case can never overflow and need no error checking
// and this one is for non-wrapping signed cases
template<class R> struct mod_impl<R, true> {
	static R mod(R v0, R v1) {
		if (v0 == std::numeric_limits<R>::min() && R(-1) == v1) 
			v0 = R();
		else
			v0 %= v1;
		return v0;
	}
};

template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct neg_impl;
template<class R> struct neg_impl<R, overflow::wrap, true> {static R neg(R v0) {return -v0;}};  // specializations for the trivial (wrapping) case
template<class R> struct neg_impl<R, overflow::wrap, false> {static R neg(R v0) {return -v0;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct neg_impl<R, OverflowMode, false> {
	static R neg(R v0) {
		if (R() != v0) 
			overflow_handle<OverflowMode>(v0, false);
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct neg_impl<R, OverflowMode, true> {
	static R neg(R v0) {
		if (v0 == std::numeric_limits<R>::min()) 
			overflow_handle<OverflowMode>(v0, true);
		else
			v0 = -v0;
		return v0;
	}
};

} // namespace detail

//! @brief Adding with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant.
//! @return (@f${v_0 + v_1}@f$)
template<overflow::mode OverflowMode, class R>
inline R add(R v0, R v1) {return detail::add_impl<R, OverflowMode>::add(v0, v1);}

//! @brief Subtracting with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant.
//! @return (@f${v_0 - v_1}@f$).
template<overflow::mode OverflowMode, class R>
inline R sub(R v0, R v1) {return detail::sub_impl<R, OverflowMode>::sub(v0, v1);}

//! @brief Multiplication with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant.
//! @return (@f${v_0 \cdot v_1}@f$).
template<overflow::mode OverflowMode, class R>
inline R mul(R v0, R v1) {return detail::mul_impl<R, OverflowMode>::mul(v0, v1);}

//! @brief Division with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant.
//! @return (@f${v_0 / v_1}@f$).
template<overflow::mode OverflowMode, class R>
inline R div(R v0, R v1) {return detail::div_impl<R, OverflowMode>::div(v0, v1);}

//! @brief Modulo with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant (ignored, as modulo can't overflow).
//! @return (@f${v_0 \mod v_1}@f$).
template<overflow::mode OverflowMode, class R>
inline R mod(R v0, R v1) {return detail::mod_impl<R>::mod(v0, v1);}

//! @brief Negation with parameterized overflow handling.
//! @tparam OverflowMode overflow handling constant.
//! @return (@f${-v_0}@f$).
template<overflow::mode OverflowMode, class R>
inline R neg(R v0) {return detail::neg_impl<R, OverflowMode>::neg(v0);}

namespace detail {
//template<class R, bool is_valid = std::numeric_limits<R>::is_integer, bool is_signed = std::numeric_limits<R>::is_signed> struct absint_impl;
//template<class R> struct absint_impl<R, true, true> {static R absint(R val) {return (val >= 0 ? val : -val);}};
//template<class R> struct absint_impl<R, true, false> {static R absint(R val) {return val;}};
//template<class R> inline R absint(R val) {return absint_impl<R>::absint(val);}

// overflow-checking rounding of integer values at specified bit according to rounding mode
template<class R, rounding::mode RoundMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct round_impl;
// discard all the bits below the specified one, validity of at_bit is checked by the driving function
template<class R, bool IsSigned> struct round_impl<R, rounding::truncated, IsSigned> {
	static R round_adj(R& val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;			// this will never overflow
		return R();
	}
};

template<class R> struct round_impl<R, rounding::nearest, false> {
	static R round_adj(R& val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;
		return (r >= div / 2) ? div : R();
	}
};

template<class R> struct round_impl<R, rounding::nearest, true> {
	static R round_adj(R& val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;
		if (r >= div / 2)
			return div;
		else if (r <= -div / 2)
			return -div;
		return R();
	}
};

template<class R, bool IsSigned> struct round_impl<R, rounding::positive, IsSigned> {
	static R round_adj(R& val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;
		if (r > 0)
			return div;
		return R();
	}
};

// for unsigned numbers rounding towards -inf is the same as truncating
template<class R> struct round_impl<R, rounding::negative, false>: public round_impl<R, rounding::truncated, false> {};

template<class R> struct round_impl<R, rounding::negative, true> {
	static R round_adj(R& val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;
		if (r < 0)
			return -div;
		return R();
	}
};

enum ovf_check {ovf_none, ovf_neg, ovf_pos};

// testing if number overflows above specified bit count
template<class R, bool IsSigned = std::numeric_limits<R>::is_signed> struct check_overflow_impl;
template<class R> struct check_overflow_impl<R, true> {
	static ovf_check check(R val, R& sat, int bit_count) {
		if (bit_count < 0 && val != R()) {
			sat = R();
			return (val > 0) ? ovf_pos : ovf_neg;
		}
		else if (bit_count < std::numeric_limits<R>::digits) {
			if (val < R()) {
				sat = R(-1) << static_cast<R>(bit_count);
				if (val < sat)
					return ovf_neg;
			}
			else {
				sat = (R(1) << static_cast<R>(bit_count)) - R(1);
				if (val > sat)
					return ovf_pos;
			}
		}
		return ovf_none;
	}
};

template<class R> struct check_overflow_impl<R, false> {
	static ovf_check check(R val, R& sat, int bit_count) {
		if (bit_count < 0 && val != R()) {
			sat = R();
			return ovf_pos;
		}
		else if (bit_count < std::numeric_limits<R>::digits) {
			sat = (R(1) << static_cast<R>(bit_count)) - R(1);
			if (val > sat)
				return ovf_pos;
		}
		return ovf_none;
	}
};

// overflow checking and handling
template<class R, overflow::mode OverflowMode> struct overflow_check_handle_impl;
template<class R> struct overflow_check_handle_impl<R, overflow::wrap> {static void check_handle(R&, int) {}};
template<class R> struct overflow_check_handle_impl<R, overflow::saturate> {
	static void check_handle(R& val, int bit_count) {
		R sat;
		if (ovf_none != check_overflow_impl<R>::check(val, sat, bit_count))
			val = sat;
	}
};

template<class R> struct overflow_check_handle_impl<R, overflow::exception> {
	static void check_handle(R& val, int bit_count) {
		R sat;
		if (ovf_none != check_overflow_impl<R>::check(val, sat, bit_count))
			throw std::overflow_error("overflow");
	}
};

} // namespace detail

//! @brief Truncate the integer value at specified bit and get rounding adjustment which should be added to
//! the truncated value in order to round it with the specified mode.
//! @tparam RoundMode rounding mode to use to obtain the adjustment.
//! @param[in,out] val value to truncate and get the adjustment for.
//! @param[in] at_bit bits below this one will be truncated, the adjustment will be one of {-1 << at_bit, 0, 1 << at_bit}.
template<rounding::mode RoundMode, class R>
inline R rounding_adjustment(R& val, int at_bit) {
	if (at_bit <= 0)
		return R();
	else if (at_bit >= std::numeric_limits<R>::digits)
		return (val = R());
	else
		return detail::round_impl<R, RoundMode>::round_adj(val, at_bit);
}

//! @brief Round the integer number below specified bit.
//! @tparam RoundMode rounding mode to use.
//! @tparam OverflowMode overflow handling mode to use if the rounding result overflows the dynamic range of integer type.
//! @param[in] val value to round.
//! @param[in] at_bit bits below this one will be rounded and cleared (if <= 0, it's a no-op).
template<rounding::mode RoundMode, overflow::mode OverflowMode, class R>
inline R round(R val, int at_bit) {
	R adj = rounding_adjustment<RoundMode>(val, at_bit);
	return add<OverflowMode>(val, adj);
}

template<rounding::mode RoundMode, class R>
inline R round(R val, int at_bit) {
	return round<RoundMode, overflow::fastest>(val, at_bit);
}

//! @brief Check if integer number overflows specified number of bits.
//! @tparam OverflowMode overflow handling mode.
//! @param[in,out] val value to check overflow in (and possibly adjust, if saturating).
//! @param[in] bit_count number of bits the @p val should fit into.
template<overflow::mode OverflowMode, class R>
inline void overflow_check_handle(R& val, int bit_count) {
	return detail::overflow_check_handle_impl<R, OverflowMode>::check_handle(val, bit_count);
}

namespace detail {
// floating point rounding
template<class F, rounding::mode RoundingMode> struct float_round_impl;
template<class F> struct float_round_impl<F, rounding::truncated> {static F round(F f) {return (f < F()) ? std::ceil(f) : std::floor(f);}};
template<class F> struct float_round_impl<F, rounding::nearest> {static F round(F f) {return (f < F()) ? std::ceil(f - F(.5)) : std::floor(f + F(.5));}};
template<class F> struct float_round_impl<F, rounding::negative> {static F round(F f) {return std::floor(f);}};
template<class F> struct float_round_impl<F, rounding::positive> {static F round(F f) {return std::ceil(f);}};

template<class R, class F, overflow::mode OverflowMode> struct float_overflow_impl {
	static R cast(F f) {
		R res = static_cast<R>(f);
		bool up;
		if ((up = (f > std::numeric_limits<R>::max())) || f < std::numeric_limits<R>::min())
			overflow_handle_impl<R, OverflowMode>::handle(res, up);
		return res;
	}
};

template<class R, class F> struct float_overflow_impl<R, F, overflow::wrap> {static R cast(F f) {return static_cast<R>(f);}};
}

//! @brief Round floating point number according to rounding::mode provided as a template parameter.
//! @tparam R type of the result.
template<class R, rounding::mode RoundingMode, overflow::mode OverflowMode, class F>
inline R rint(F f) {return detail::float_overflow_impl<R, F, OverflowMode>::cast(detail::float_round_impl<F, RoundingMode>::round(f));}

template<class R, class F>
inline R rint(F f, rounding::mode rm, overflow::mode om) {
	F rnd;
	switch (rm) {
	case rounding::truncated: 	rnd = detail::float_round_impl<F, rounding::truncated>::round(f); break;
	case rounding::nearest:		rnd = detail::float_round_impl<F, rounding::nearest>::round(f); break;
	case rounding::negative:	rnd = detail::float_round_impl<F, rounding::negative>::round(f); break;
	case rounding::positive:	rnd = detail::float_round_impl<F, rounding::positive>::round(f); break;
	default: throw std::invalid_argument("unknown rounding mode");
	}
	R res;
	switch (om) {
	case overflow::wrap:		res = detail::float_overflow_impl<R, F, overflow::wrap>::cast(rnd); break;
	case overflow::saturate:	res = detail::float_overflow_impl<R, F, overflow::saturate>::cast(rnd); break;
	case overflow::exception:	res = detail::float_overflow_impl<R, F, overflow::exception>::cast(rnd); break;
	default: throw std::invalid_argument("unknown overflow mode");
	}
	return res;
}

//! @return Greatest Common Divisor calculated using Euclid's algorithm
template<class Int>
Int gcd(Int a, Int b) {
	while (true) {
		a = a % b;
		if (0 == a)
			return b;
		b = b % a;
		if (0 == b)
			return a;
	}
}

} // namespace dsp

#endif /* DSP_INTMATH_H_INCLUDED */
