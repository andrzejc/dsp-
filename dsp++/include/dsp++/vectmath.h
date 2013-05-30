/*!
 * @file dsp++/vectmath.h
 * @brief Basic vector mathematical operations, with some optimizations if possible.
 */

#include <dsp++/config.h>
#include <dsp++/export.h>

#include <complex>			// for std::complex
#include <cstddef>			// for size_t

namespace dsp { 

	namespace simd {

		/*!
		 * @brief Piecewise multiplication of SIMD-aligned and padded vectors of equal length
		 * (@f$ res = \left \{ a_0\cdot b_0, a_1\cdot b_1, \cdots  a_{len-1}\cdot b_{len-1} \right \} @f$).
		 * @param [out] res output vector (SIMD-aligned and padded) (len)
		 * @param [in] a first multiplicand (SIMD-aligned and padded) (len)
		 * @param [in] b second multiplicand (SIMD-aligned and padded) (len)
		 * @param [in] len length of vectors.
		 */
		DSPXX_API void mul(float* res, const float* a, const float* b, size_t len);

		//! @copydoc mul(float*, const float*, const float*, size_t)
		DSPXX_API void mul(std::complex<float>* res, const std::complex<float>* a, const std::complex<float>* b, size_t len);

		/*!@brief Multiply vector a by scalar as and write result to res.
		 * @param [out] res output vector (SIMD-aligned and padded) (len).
		 * @param [in] a vector operand (SIMD-aligned and padded) (len).
		 * @param [in] s scalar operand (len).
		 * @param [in] len length of vectors.
		 */
		DSPXX_API void mul(float* res, const float* a, float s, size_t len);

		/*!
		 * @brief Dot product of SIMD-aligned and padded vectors of equal length
		 * (@f$ \sum_{i = 0}^{len - 1} a_i\cdot b_i @f$).
		 * @return dot product
		 * @param [in] a first multiplicand (SIMD-aligned and padded) (len)
		 * @param [in] b second multiplicand (SIMD-aligned and padded) (len)
		 * @param [in] len length of vectors.
		 */
		DSPXX_API float dot(const float* a, const float* b, size_t len);

		//! @copydoc dot(const float*, const float*, size_t)
		DSPXX_API std::complex<float> dot(const std::complex<float>* a, const std::complex<float>* b, size_t len);

		// TODO implement other interesting SIMD-accelerated vector ops
	}

	/*!@brief Naïve implementation of piecewise vector multiplication.
	 * @param[out] res output vector (may be the same as input) (len).
	 * @param[in] a vector operand (len).
	 * @param[in] b vector operand (len).
	 * @param len length of operands.
	 */
	template<class T>
	inline void mul(T* res, const T* a, const T* b, size_t len)
	{
		for (size_t i = 0; i < len; ++i, ++res, ++a, ++b)
			*res = *a * *b;
	}

	/*!@brief Naïve implementation of dot product.
	 * @param[in] a vector operand (len).
	 * @param[in] b vector operand (len).
	 * @param len length of operands.
	 * @return dot product of operands (@f$ a \cdot b @f$).
	 */
	template<class T>
	inline T dot(const T* a, const T* b, size_t len)
	{
		T res = T();
		for (size_t i = 0; i < len; ++i, ++a, ++b)
			res += *a * *b;
		return res;
	}

	/*!@brief Naïve implementation of vector @f$\times@f$ scalar multiplication. Multiply (scale) vector a by scalar s, write result to res.
	 * @param[out] res output vector (may be the same as input) (len).
	 * @param[in] a vector to scale (len).
	 * @param[in] s scalar value to multiply vector by.
	 * @param[in] len length of vectors.
	 */
	template<class T>
	inline void mul(T* res, const T* a, T s, size_t len)
	{
		for (size_t i = 0; i < len; ++i, ++res, ++a)
			*res = *a * s;
	}
} 
