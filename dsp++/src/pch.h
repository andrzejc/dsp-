/**
 * @file pch.h
 * Prcompiler header support
 * @author Andrzej Ciarkowski <andrzej.ciarkowski@gmail.com>
 */
#pragma once

#include <dsp++/config.h>


#ifdef __cplusplus

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

#include <dsp++/concept_checks.h>

#if !DSP_BOOST_DISABLED
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/circular_buffer.hpp>
#endif // !DSP_BOOST_DISABLED

#if !DSP_FFTW_DISABLED
#include <fftw3.h>
#endif

#if !DSP_SNDFILE_DISABLED
#include <sndfile.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32


#endif // __cplusplus

