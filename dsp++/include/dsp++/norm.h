/// @file dsp++/norm.h
/// @brief Algorithms for computing vector norms.
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_NORM_H_INCLUDED
#define DSP_NORM_H_INCLUDED
#pragma once

#include <dsp++/complex.h>

#include <algorithm>
#include <cmath>

namespace dsp {

/// @brief Specify norm kind - absolute or relative.
namespace norm { enum type {
	abs,	///< Absolute 
	rel
}; }

namespace detail {
	template<norm::type t, class T>
	struct error_impl;

	template<class T>
	struct error_impl<norm::abs, T> 
	{
		typename dsp::remove_complex<T>::type operator()(const T& a, const T& b) 
		{
			using namespace std;
			return abs(a - b);
		}
	};

	template<class T>
	struct error_impl<norm::rel, T> 
	{
		typename dsp::remove_complex<T>::type operator()(const T& a, const T& b) 
		{
			using namespace std;
			return abs(a - b) / abs(b);
		}
	};
}

/// @brief Calculate error (absolute difference) between two values, either absolute or relative.
/// Absolute error is calculated as: \f$\varepsilon_{abs} = \left|a - b\right|\f$; relative error is: 
/// \f$\varepsilon_{rel} = {{\left|a - b\right|}\over{\left|b\right|}}\f$.
/// @tparam t Type of error - either absolute or relative.
/// @tparam T Type of values being compared.
/// @param a value to compare.
/// @param b value to compare.
/// @return either absolute or relative error between the two values @p a and @p b.
template<norm::type t, class T>
inline typename dsp::remove_complex<T>::type error(const T& a, const T& b) 
{ return detail::error_impl<t, T>()(a, b); }


template<class InputIterator>
inline typename dsp::remove_complex<typename std::iterator_traits<InputIterator>::value_type>::type norm_1(InputIterator begin, InputIterator end)
{
	typedef typename std::iterator_traits<InputIterator>::value_type T;
	typedef typename dsp::remove_complex<typename std::iterator_traits<InputIterator>::value_type>::type R;
	using namespace std;
	R res = R();
	while (begin != end)
		res += abs(*begin++); // this could be: error<norm::abs>(*begin++, T()) but let's not get paranoid
	return res;
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_1(const T* a, size_t len)
{
	return norm_1(a, a + len);
}


template<norm::type t, class T>
typename dsp::remove_complex<T>::type norm_1(const T* a, const T* b, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a, ++b)
		res += error<t>(*a,*b);
	return res;
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_2(const T* a, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a) {
		R m = error<norm::abs>(*a, T());
		res += m * m;
	}
	return sqrt(res);
}

template<norm::type t, class T>
typename dsp::remove_complex<T>::type norm_2(const T* a, const T* b, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a, ++b) {
		R m = error<t>(*a, *b);
		res += m * m;
	}
	return sqrt(res);
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_inf(const T* a, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a)
		res = max(res, error<norm::abs>(*a, T()));
	return res;
}

template<norm::type t, class T>
inline typename dsp::remove_complex<T>::type norm_inf(const T* a, const T* b, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a, ++b)
		res = max(res, error<t>(*a, *b));
	return res;
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_rms(const T* a, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a) {
		R m = error<norm::abs>(*a, T());
		res += m * m;
	}
	return sqrt(res / len);
}

template<norm::type t, class T>
typename dsp::remove_complex<T>::type norm_rms(const T* a, const T* b, size_t len) 
{
	typedef typename dsp::remove_complex<T>::type R;
	using namespace std;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a, ++b) {
		R m = error<t>(*a, *b);
		res += m * m;
	}
	return sqrt(res / len);
}

}

#endif /* DSP_NORM_H_INCLUDED */
