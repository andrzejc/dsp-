/// @file dsp++/export.h
/// @brief Definition of @c DSPXX_API macro - dllimport/export/visibility stuff.
/// @author Andrzej Ciarkowski <andrzej.ciarkowski@gmail.com>
#pragma once
/// @def DSPXX_API
/// @brief Macro adding compiler-specific attributes which control linking or visibility of
/// library symbols. Should be used before all public symbol declarations (functions and variables)
/// or right after "class" or "struct" keyword.
#if !defined(DSPXX_API)
  /// @def DSPXX_STATIC
  /// @brief Defined by the build system during compilation of statically-linked libdsp++
  /// and should be defined by the consumers using it (this is automatically added to the
  /// interface of static libdsp++ when it is used via exported CMake target config files).
  #if defined(DSPXX_STATIC)
    #define DSPXX_API
  #else
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
  #endif // !DSPXX_STATIC
#endif // DSPXX_API
