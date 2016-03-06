/// @file dsp++/compat.h
/// @brief Language compatibility macros & helpers
/// @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
#ifndef DSP_COMPAT_H_INCLUDED
#define DSP_COMPAT_H_INCLUDED

#include <algorithm>

namespace dsp {

#if DSP_CXX_CONFORM_CXX11
	using std::is_sorted;
	using std::is_sorted_until;
#else // !DSP_CXX_CONFORM_CXX11

template<class ForwardIt>
ForwardIt is_sorted_until(ForwardIt first, ForwardIt last)
{
	if (first != last) {
		ForwardIt next = first;
		while (++next != last) {
			if (*next < *first)
				return next;
			first = next;
		}
	}
	return last;
}

template<class ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last)
{
	return dsp::is_sorted_until(first, last) == last;
}

#endif // !DSP_CXX_CONFORM_CXX11

}

#endif // DSP_COMPAT_H_INCLUDED
