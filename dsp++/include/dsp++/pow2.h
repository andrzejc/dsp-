/*!
 * @file dsp++/pow2.h
 * @brief Utilities for finding nearest power of 2 and checking whether a number is one.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_POW2_H_INCLUDED
#define DSP_POW2_H_INCLUDED

#include <limits>
#include <climits>
#include <cstddef>

namespace dsp {

/*!
 * @brief Find the least power of two that is not less than a number k.
 * @param k the number.
 * @return the least power of that is not less than k.
 */
template<class T>
T nextpow2(T k)
{
	if (k <= 0)
		return 1;
	k--;
	for (size_t i = 1; i < sizeof(T) * CHAR_BIT; i <<= 1)
		k = k | k >> i;
	return k + 1;
}

template<class T>
bool ispow2(T k)
{
	if (std::numeric_limits<T>::is_signed)
		return (k) && ((k & -k) == k);
	else
		return (k) && (0 == (k & (k - 1)));
}

}

#endif /* DSP_POW2_H_INCLUDED */
