/*!
 * @file dsp++/stdint.h
 * @brief Pull std:: int types into dsp namespace and add some utilities like @c select_int,
 * @c signed_of and @c unsigned_of.
 */
#pragma once
#include <cstdint>

namespace dsp {

namespace detail {

template<class T> struct next_int {typedef void type;};
template<> struct next_int<char> {typedef short type;};
template<> struct next_int<short> {typedef int type;};
template<> struct next_int<int> {typedef long type;};
template<> struct next_int<long> {typedef long long type;};
template<> struct next_int<unsigned char> {typedef unsigned short type;};
template<> struct next_int<unsigned short> {typedef unsigned int type;};
template<> struct next_int<unsigned int> {typedef unsigned long type;};
template<> struct next_int<unsigned long> {typedef unsigned long long type;};

template<int size, class T, bool is_same_size = (size == 8 * sizeof(T))> struct select_sized_int;
template<int size, class T> struct select_sized_int<size, T, true> { typedef T type; };
template<int size, class T> struct select_sized_int<size, T, false> { typedef typename select_sized_int<size, typename next_int<T>::type>::type type; };

} // namespace detail

/// @brief Compile-time bit-sized int-type detection: @code{.cpp} typename select_int<size, signed>::type @endcode.
template<int size,  bool sign> struct select_int;
template<int size> struct select_int<size, true> {typedef typename detail::select_sized_int<size, char>::type type;};
template<int size> struct select_int<size, false> {typedef typename detail::select_sized_int<size, unsigned char>::type type;};

template<int size, bool sign>
using select_int_t = typename select_int<size, sign>::type;

using std::int8_t;
using std::uint8_t;
using std::int16_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::int64_t;
using std::uint64_t;

/// @brief Declare unsigned int type counterpart: @code{.cpp} typename unsigned_of<int>::type @endcode.
template<class T> struct unsigned_of;
template<> struct unsigned_of<signed char> {typedef unsigned char type;};
/// @note stdc++ 3.9.1 [basic.fundamental]: char != signed char, these are
/// different types!
template<> struct unsigned_of<char> {typedef unsigned char type;};
template<> struct unsigned_of<unsigned char> {typedef unsigned char type;};
template<> struct unsigned_of<signed short> {typedef unsigned short type;};
template<> struct unsigned_of<unsigned short> {typedef unsigned short type;};
template<> struct unsigned_of<signed int> {typedef unsigned int type;};
template<> struct unsigned_of<unsigned int> {typedef unsigned int type;};
template<> struct unsigned_of<signed long> {typedef unsigned long type;};
template<> struct unsigned_of<unsigned long> {typedef unsigned long type;};
template<> struct unsigned_of<signed long long> {typedef unsigned long long type;};
template<> struct unsigned_of<unsigned long long> {typedef unsigned long long type;};
template<typename T>
using unsigned_of_t = typename unsigned_of<T>::type;

/// @brief Declare signed int type counterpart: @code{.cpp} typename signed_of<unsigned>::type @endcode.
template<class T> struct signed_of;
template<> struct signed_of<signed char> {typedef signed char type;};
/// @note stdc++ 3.9.1 [basic.fundamental]: char != signed char, these are
/// different types!
template<> struct signed_of<char> {typedef signed char type;};
template<> struct signed_of<unsigned char> {typedef signed char type;};
template<> struct signed_of<signed short> {typedef signed short type;};
template<> struct signed_of<unsigned short> {typedef signed short type;};
template<> struct signed_of<signed int> {typedef signed int type;};
template<> struct signed_of<unsigned int> {typedef signed int type;};
template<> struct signed_of<signed long> {typedef signed long type;};
template<> struct signed_of<unsigned long> {typedef signed long type;};
template<> struct signed_of<signed long long> {typedef signed long long type;};
template<> struct signed_of<unsigned long long> {typedef signed long long type;};
template<typename T>
using signed_of_t = typename signed_of<T>::type;

}
