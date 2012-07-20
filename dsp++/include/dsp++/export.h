/*!
 * @file dsp++/export.h
 * @brief dllimport/export/visibility stuff.
 * @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
 */

#ifndef DSP_EXPORT_H_INCLUDED
#define DSP_EXPORT_H_INCLUDED

#if defined _WIN32 || defined __CYGWIN__
  #ifdef DSPXX_EXPORTS
    #ifdef __GNUC__
      #define DSPXX_API __attribute__ ((dllexport))
    #else
      #define DSPXX_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define DSPXX_API __attribute__ ((dllimport))
    #else
      #define DSPXX_API __declspec(dllimport)
    #endif
  #endif
#else
  #if __GNUC__ >= 4
    #define DSPXX_API __attribute__ ((visibility ("default")))
  #else
    #define DSPXX_API
  #endif
#endif

#endif /* DSP_EXPORT_H_INCLUDED */
