/*!
 * @file dsp++/utility.h
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_UTILITY_H_INCLUDED
#define DSP_UTILITY_H_INCLUDED
#pragma once

#include <dsp++/config.h>

#if !DSP_BOOST_DISABLED
#include <cstring>
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#endif

namespace dsp {

#if !DSP_BOOST_DISABLED

template<class Elem>
typename boost::enable_if<boost::has_trivial_assign<Elem>, void>::type inline
delay(Elem* vec, size_t N)
{std::memmove(vec + 1, vec, (N - 1) * sizeof(Elem));}

template<class Elem>
typename boost::disable_if<boost::has_trivial_assign<Elem>, void>::type inline
delay(Elem* vec, size_t N)
{for (size_t i = N - 1; i > 0; --i) vec[i] = vec[i - 1];}}

#else // DSP_BOOST_DISABLED

template<class Elem>
void inline
delay(Elem* vec, size_t N)
{for (size_t i = N - 1; i > 0; --i) vec[i] = vec[i - 1];}}

#endif // DSP_BOOST_DISABLED

#endif /* DSP_UTILITY_H_INCLUDED */
