/*!
 * @file dsp++/utility.h
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_UTILITY_H_INCLUDED
#define DSP_UTILITY_H_INCLUDED


#include <cstring>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

namespace dsp {

template<class Elem>
typename boost::enable_if<boost::has_trivial_assign<Elem>, void>::type inline
delay(Elem* vec, size_t N)
{memmove(vec + 1, vec, (N - 1) * sizeof(Elem));}

template<class Elem>
typename boost::disable_if<boost::has_trivial_assign<Elem>, void>::type inline
delay(Elem* vec, size_t N)
{for (size_t i = N - 1; i > 0; --i) vec[i] = vec[i - 1];}

}

#endif /* DSP_UTILITY_H_INCLUDED */
