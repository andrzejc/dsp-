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
		near_even, 				//!< Round towards the nearest value, but exactly-half values are rounded towards even values. This mode has more balance than the classic mode.
		near_odd,				//!< Round towards the nearest value, but exactly-half values are rounded towards odd values. This mode has as much balance as the near_even mode, but preserves more information.
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
struct overflow_impl {static void handle(R&, bool) {}};

template<class R> struct overflow_impl<R, overflow::saturate>
{static void handle(R& val, bool up) {val = (up ? std::numeric_limits<R>::max() : std::numeric_limits<R>::min());}};

template<class R> struct overflow_impl<R, overflow::exception>
{static void handle(R&, bool) {throw std::overflow_error("overflow");}};

} // namespace detail

//! @brief Handle overflow detected by higher-level algorithm.
//! @tparam OverflowMode handling mode
//! @tparam R type of variable in which where overflow occurred.
//! @param[in,out] val overflowed value, may be adjusted upon return.
//! @param[in] up specifies whether overflow occurred in higher or lower direction (overflow/underflow).
template<overflow::mode OverflowMode, class R>
inline void handle_overflow(R& val, bool up) {detail::overflow_impl<R, OverflowMode>::handle(val, up);}

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
			handle_overflow<OverflowMode>(v0, true);
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
			handle_overflow<OverflowMode>(v0, up);
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
			handle_overflow<OverflowMode>(v0, false);
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
			handle_overflow<OverflowMode>(v0, up);
		return v0;
	}
};

template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct mul_impl;
template<class R> struct mul_impl<R, overflow::wrap, true> {static R mul(R v0, R v1) {return v0 * v1;}};  // specializations for the trivial (wrapping) case
template<class R> struct mul_impl<R, overflow::wrap, false> {static R mul(R v0, R v1) {return v0 * v1;}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// this specialization covers all non-wrapping unsigned cases
template<class R, overflow::mode OverflowMode> struct mul_impl<R, OverflowMode, false> {
	static R mul(R v0, R v1) {
		bool ovf = (v0 != 0 && v1 != 0 && v0 > std::numeric_limits<R>::max() / v1);
		v0 *= v1;
		if (ovf)
			handle_overflow<OverflowMode>(v0, true);
		return v0;
	}
};
// and this one is for non-wrapping signed cases
template<class R, overflow::mode OverflowMode> struct mul_impl<R, OverflowMode, true> {
	static R mul(R v0, R v1) {
		bool ovf = false;
		bool up;
		if (v0 > 0) {
			if (v1 > 0) {
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
			if (v1 > 0) {
				if (v0 < std::numeric_limits<R>::min() / v1) {
					ovf = true;
					up = false;
				}
			}
			else {
				if ((v0 != 0) && (v1 < std::numeric_limits<R>::max() / v0)) {
					ovf = true;
					up = true;
				}
			}
		}
		v0 *= v1;
		if (ovf)
			handle_overflow<OverflowMode>(v0, up);
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

namespace detail {
//template<class R, bool is_valid = std::numeric_limits<R>::is_integer, bool is_signed = std::numeric_limits<R>::is_signed> struct absint_impl;
//template<class R> struct absint_impl<R, true, true> {static R absint(R val) {return (val >= 0 ? val : -val);}};
//template<class R> struct absint_impl<R, true, false> {static R absint(R val) {return val;}};
//template<class R> inline R absint(R val) {return absint_impl<R>::absint(val);}

// overflow-checking rounding of integer values at specified bit according to rounding mode
template<class R, rounding::mode RoundMode, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct round_impl;
// discard all the bits below the specified one, validity of at_bit is checked by the driving function
template<class R, overflow::mode OverflowMode, bool IsSigned> struct round_impl<R, rounding::truncated, OverflowMode, IsSigned> {
	static R round(R val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		val -= r;			// this will never overflow
		return val;
	}
};

template<class R, overflow::mode OverflowMode> struct round_impl<R, rounding::nearest, OverflowMode, false> {
	static R round(R val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		if (0 == r)
			return val;
		val -= r;
		if (r >= div / 2)
			val = add<OverflowMode>(val, div);
		return val;
	}
};

template<class R, overflow::mode OverflowMode> struct round_impl<R, rounding::nearest, OverflowMode, true> {
	static R round(R val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		if (0 == r)
			return val;
		val -= r;
		if (r >= div / 2)
			val = add<OverflowMode>(val, div);
		else if (r <= -div / 2)
			val = sub<OverflowMode>(val, div);
		return val;
	}
};


template<class R, overflow::mode OverflowMode, bool IsSigned> struct round_impl<R, rounding::positive, OverflowMode, IsSigned> {
	static R round(R val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		if (0 == r)
			return val;
		val -= r;
		if (r > 0)
			val = add<OverflowMode>(val, div);
		return val;
	}
};

// for unsigned numbers rounding towards -inf is the same as truncating
template<class R, overflow::mode OverflowMode> struct round_impl<R, rounding::negative, OverflowMode, false>: public round_impl<R, rounding::truncated, OverflowMode, false> {};

template<class R, overflow::mode OverflowMode> struct round_impl<R, rounding::negative, OverflowMode, true> {
	static R round(R val, int at_bit) {
		R div = R(1) << static_cast<R>(at_bit);
		R r = val % div;
		if (0 == r)
			return val;
		val -= r;
		if (r < 0)
			val = sub<OverflowMode>(val, div);
		return val;
	}
};

// testing if number overflows above specified bit count
template<class R, overflow::mode OverflowMode, bool IsSigned = std::numeric_limits<R>::is_signed> struct check_overflow_impl;
template<class R> struct check_overflow_impl<R, overflow::wrap, true> {static void check(R&, int) {}};  // specializations for the trivial (wrapping) case
template<class R> struct check_overflow_impl<R, overflow::wrap, false> {static void check(R&, int) {}}; // we need both signed and unsigned variants so that we don't get compilation errors due to ambiguous template resolution
// overflow checking with saturation, unsigned case
template<class R> struct check_overflow_impl<R, overflow::saturate, false> {
	static void check(R& val, int bit_count) {
		if (bit_count < 0 && val != R())
			val = 0;
		else if (bit_count < std::numeric_limits<R>::digits) {
			R max = (R(1) << static_cast<R>(bit_count)) - R(1);
			if (val > max)
				val = max;
		}
	}
};
// overflow checking with saturation, signed case
template<class R> struct check_overflow_impl<R, overflow::saturate, true> {
	static void check(R& val, int bit_count) {
		if (bit_count < 0 && val != R())
			val = 0;
		else if (bit_count < std::numeric_limits<R>::digits) {
			if (val < R()) {
				R min = R(-1) << static_cast<R>(bit_count);
				if (val < min)
					val = min;
			}
			else {
				R max = (R(1) << static_cast<R>(bit_count)) - R(1);
				if (val > max)
					val = max;
			}
		}
	}
};
// overflow checking with exception, unsigned case
template<class R> struct check_overflow_impl<R, overflow::exception, false> {
	static void check(R& val, int bit_count) {
		if (bit_count < 0 && val != R())
			throw std::overflow_error("overflow");
		else if (bit_count < std::numeric_limits<R>::digits) {
			R max = (R(1) << static_cast<R>(bit_count)) - R(1);
			if (val > max)
				throw std::overflow_error("overflow");
		}
	}
};
// overflow checking with exception, signed case
template<class R> struct check_overflow_impl<R, overflow::exception, true> {
	static void check(R& val, int bit_count) {
		if (bit_count < 0 && val != R())
			throw std::overflow_error("overflow");
		else if (bit_count < std::numeric_limits<R>::digits) {
			if (val < R()) {
				R min = R(-1) << static_cast<R>(bit_count);
				if (val < min)
					throw std::overflow_error("overflow");
			}
			else {
				R max = (R(1) << static_cast<R>(bit_count)) - R(1);
				if (val > max)
					throw std::overflow_error("overflow");
			}
		}
	}
};

} // namespace detail

//! @brief Round the integer number below specified bit.
//! @tparam RoundMode rounding mode to use.
//! @tparam OverflowMode overflow handling mode to use if the rounding result overflows the dynamic range of integer type.
//! @param[in] val value to round.
//! @param[in] at_bit bits below this one will be rounded and cleared (if <= 0, it's a no-op).
template<rounding::mode RoundMode, overflow::mode OverflowMode, class R>
inline R round(R val, int at_bit) {
	if (at_bit <= 0)
		return val;
	else if (at_bit >= std::numeric_limits<R>::digits && val != 0) {
		bool up = (val > 0);
		val = 0;
		handle_overflow<OverflowMode>(val, up);
	}
	else
		val = detail::round_impl<R, RoundMode, OverflowMode>::round(val, at_bit);
	return val;
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
inline void check_overflow(R& val, int bit_count) {
	return detail::check_overflow_impl<R, OverflowMode>::check(val, bit_count);
}

} // namespace dsp

#endif /* DSP_INTMATH_H_INCLUDED */
