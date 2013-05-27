/*!
 * @file dsp++/vectmath.h
 * @brief Basic vector mathematical operations, with some optimizations if possible.
 */

#include <dsp++/config.h>
#include <dsp++/export.h>

#include <complex>
#include <cstddef>			// for size_t

namespace dsp { 

	namespace simd {

		/*!
		 * @brief Piecewise multiplication of SIMD-aligned and padded vectors of equal length
		 * (@f$ res = \left \{ a_0\cdot b_0, a_1\cdot b_1, \cdots  a_{len-1}\cdot b_{len-1} \right \} @f$).
		 * @param [out] res Output vector (len)
		 * @param [in] a First multiplicand (len)
		 * @param [in] b Second multiplicand (len)
		 * @param [in] len Length of all vectors, must be a multiply of dsp::simd::alignment(), padded with 0s as necessary.
		 */
		DSPXX_API void mul(float* res, const float* a, const float* b, size_t len);
		//! @copydoc mul(float*, const float*, const float*, size_t)
		DSPXX_API void mul(std::complex<float>* res, const std::complex<float>* a, const std::complex<float>* b, size_t len);

		/*!
		 * @brief Dot product of SIMD-aligned and padded vectors of equal length
		 * (@f$ \sum_{i = 0}^{len - 1} a_i\cdot b_i @f$).
		 * @return dot product
		 * @param [in] a First multiplicand (len)
		 * @param [in] b Second multiplicand (len)
		 * @param [in] len Length of all vectors, must be a multiply of dsp::simd::alignment(), padded with 0s as necessary.
		 */
		DSPXX_API float dot(const float* a, const float* b, size_t len);
		//! @copydoc dot(const float*, const float*, size_t)
		DSPXX_API std::complex<float> dot(const std::complex<float>* a, const std::complex<float>* b, size_t len);

	}

	//! @brief Naïve implementation of piecewise vector multiplication.
	template<class T>
	inline void mul(T* res, const T* a, const T* b, size_t len)
	{
		for (size_t i = 0; i < len; ++i, ++res, ++a, ++b)
			*res = *a * *b;
	}

	//! @brief Naïve implementation of dot product.
	template<class T>
	inline T dot(const T* a, const T* b, size_t len)
	{
		T res = T();
		for (size_t i = 0; i < len; ++i, ++a, ++b)
			res += *a * *b;
		return res;
	}

} 
