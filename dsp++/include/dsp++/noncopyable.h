/*!
 * @file dsp++/noncopyable.h
 * @brief Reimplementation of boost::noncopyable used when boost is diabled.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_NONCOPYABLE_H_INCLUDED
#define DSP_NONCOPYABLE_H_INCLUDED

#include <dsp++/config.h>

#if !DSP_BOOST_DISABLED
#include <boost/noncopyable.hpp>
#endif

namespace dsp {

#if !DSP_BOOST_DISABLED
typedef boost::noncopyable noncopyable;
#else // DSP_BOOST_DISABLED

class noncopyable {
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&);
protected:
   noncopyable() {}
   ~noncopyable() {}
};

#endif // DSP_BOOST_DISABLED

}

#endif /* DSP_NONCOPYABLE_H_INCLUDED */

