/*
 * norm.h
 *
 *  Created on: 31-05-2013
 *      Author: Andrzej
 */

#ifndef DSP_NORM_H_INCLUDED
#define DSP_NORM_H_INCLUDED
#pragma once

#include <dsp++/complex.h>

#include <algorithm>
#include <cmath>

namespace dsp {

enum norm_tag {
	norm_abs,
	norm_rel
};

namespace detail {
	template<norm_tag t, class T>
	struct norm_error_helper;

	template<class T>
	struct norm_error_helper<norm_abs, T> {
		typename dsp::remove_complex<T>::type operator()(const T& a, const T& b) {
			using std::abs;
			return abs(a - b);
		}
	};

	template<class T>
	struct norm_error_helper<norm_rel, T> {
		typename dsp::remove_complex<T>::type operator()(const T& a, const T& b) {
			using std::abs;
			return abs(a - b) / abs(b);
		}
	};
}


template<class T>
inline typename dsp::remove_complex<T>::type norm_1(const T* a, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::abs;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a)
		res += abs(*a);
	return res;
}

template<norm_tag t, class T>
typename dsp::remove_complex<T>::type norm_1(const T* a, const T* b, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	R res = R();
	dsp::detail::norm_error_helper<t, T> f;
	for (size_t i = 0; i < len; ++i, ++a, ++b)
		res += f(*a,*b);
	return res;
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_2(const T* a, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::abs; using std::sqrt;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a) {
		R m = abs(*a);
		res += m * m;
	}
	return sqrt(res);
}

template<norm_tag t, class T>
typename dsp::remove_complex<T>::type norm_2(const T* a, const T* b, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::sqrt;
	R res = R();
	dsp::detail::norm_error_helper<t, T> f;
	for (size_t i = 0; i < len; ++i, ++a, ++b) {
		R m = f(*a, *b);
		res += m * m;
	}
	return sqrt(res);
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_inf(const T* a, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::max; using std::abs;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a)
		res = max(res, abs(*a));
	return res;
}

template<norm_tag t, class T>
inline typename dsp::remove_complex<T>::type norm_inf(const T* a, const T* b, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::max;
	R res = R();
	dsp::detail::norm_error_helper<t, T> f;
	for (size_t i = 0; i < len; ++i, ++a, ++b)
		res = max(res, f(*a, *b));
	return res;
}

template<class T>
inline typename dsp::remove_complex<T>::type norm_rms(const T* a, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::abs; using std::sqrt;
	R res = R();
	for (size_t i = 0; i < len; ++i, ++a) {
		R m = abs(*a);
		res += m * m / len;
	}
	return sqrt(res);
}

template<norm_tag t, class T>
typename dsp::remove_complex<T>::type norm_rms(const T* a, const T* b, size_t len) {
	typedef typename dsp::remove_complex<T>::type R;
	using std::sqrt;
	R res = R();
	dsp::detail::norm_error_helper<t, T> f;
	for (size_t i = 0; i < len; ++i, ++a, ++b) {
		R m = f(*a, *b);
		res += m * m / len;
	}
	return sqrt(res);
}

}

#endif /* DSP_NORM_H_INCLUDED */
