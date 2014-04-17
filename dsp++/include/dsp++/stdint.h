/*!
 * @file dsp++/stdint.h
 * @brief Wrapper/reimplementation of part of C++ cstdint or C99 stdint.h headers (fixed-size typedefs).
 */
#ifndef DSP_STDINT_H_INCLUDED
#define DSP_STDINT_H_INCLUDED
#pragma once

#if (__cplusplus >= 201103L) || (defined(_MSC_VER) && (_MSC_VER >= 1600))
#include <cstdint>
#define DSP_STDINT(type) std:: ## type
#elif (__STDC_VERSION__ >= 199901L) || defined(__posix__)
#include <stdint.h>
#define DSP_STDINT(type) :: ## type
#endif

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

	template<int size, class T, bool is_same_size = (size == 8*sizeof(T))> struct select_sized_int;
	template<int size, class T> struct select_sized_int<size, T, true> {typedef T type;};
	template<int size, class T> struct select_sized_int<size, T, false> {typedef typename select_sized_int<size, typename next_int<T>::type>::type type;};

} // namespace detail

template<int size,  bool sign> struct select_int;
template<int size> struct select_int<size, true> {typedef typename detail::select_sized_int<size, char>::type type;};
template<int size> struct select_int<size, false> {typedef typename detail::select_sized_int<size, unsigned char>::type type;};


#ifdef DSP_STDINT
	using DSP_STDINT(int8_t);
	using DSP_STDINT(uint8_t);
	using DSP_STDINT(int16_t);
	using DSP_STDINT(uint16_t);
	using DSP_STDINT(int32_t);
	using DSP_STDINT(uint32_t);
	using DSP_STDINT(int64_t);
	using DSP_STDINT(uint64_t);
# undef DSP_STDINT
#else

typedef select_int<8, true>::type int8_t;
typedef select_int<8, false>::type uint8_t;
typedef select_int<16, true>::type int16_t;
typedef select_int<16, false>::type uint16_t;
typedef select_int<32, true>::type int32_t;
typedef select_int<32, false>::type uint32_t;
typedef select_int<64, true>::type int64_t;
typedef select_int<64, false>::type uint64_t;

#endif

template<class T> struct unsigned_of;
template<> struct unsigned_of<signed char> {typedef unsigned char type;};
template<> struct unsigned_of<unsigned char> {typedef unsigned char type;};
template<> struct unsigned_of<signed short> {typedef unsigned short type;};
template<> struct unsigned_of<unsigned short> {typedef unsigned short type;};
template<> struct unsigned_of<signed int> {typedef unsigned int type;};
template<> struct unsigned_of<unsigned int> {typedef unsigned int type;};
template<> struct unsigned_of<signed long> {typedef unsigned long type;};
template<> struct unsigned_of<unsigned long> {typedef unsigned long type;};
template<> struct unsigned_of<signed long long> {typedef unsigned long long type;};
template<> struct unsigned_of<unsigned long long> {typedef unsigned long long type;};

template<class T> struct signed_of;
template<> struct signed_of<signed char> {typedef signed char type;};
template<> struct signed_of<unsigned char> {typedef signed char type;};
template<> struct signed_of<signed short> {typedef signed short type;};
template<> struct signed_of<unsigned short> {typedef signed short type;};
template<> struct signed_of<signed int> {typedef signed int type;};
template<> struct signed_of<unsigned int> {typedef signed int type;};
template<> struct signed_of<signed long> {typedef signed long type;};
template<> struct signed_of<unsigned long> {typedef signed long type;};
template<> struct signed_of<signed long long> {typedef signed long long type;};
template<> struct signed_of<unsigned long long> {typedef signed long long type;};

}

#endif // DSP_STDINT_H_INCLUDED
