/// @file dsp++/noncopyable.h
/// @brief Import or reimplementation of boost::noncopyable used when boost is diabled.
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_NONCOPYABLE_H_INCLUDED
#define DSP_NONCOPYABLE_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_BOOST_DISABLED

#include <boost/noncopyable.hpp>
namespace dsp {using boost::noncopyable;}

#else // DSP_BOOST_DISABLED

namespace dsp {
/// @brief Trivial reimplementation of "noncopyable" idiom.
class noncopyable {
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&);
protected:
   noncopyable() {}
   ~noncopyable() {}
};
}

#endif // DSP_BOOST_DISABLED

#endif /* DSP_NONCOPYABLE_H_INCLUDED */

