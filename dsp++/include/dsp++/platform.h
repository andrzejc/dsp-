/**
 * @file dsp++/platform.h
 * @brief Preprocessor macros which unify the way we determine for which platform we're building.
 * @todo Compile-time detection of ARM targets and their variants.
 */
#pragma once
#include <dsp++/export.h>

#ifndef DOXYGEN_RUNNING
// All the tested macros are listed on
// http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
#if defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
# define DSPXX_ARCH_IA64
# define DSPXX_ENDIAN_LITTLE
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
# define DSPXX_ARCH_FAMILY_PPC
# if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
	  defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
#  define DSPXX_ARCH_PPC64
# else
#  define DSPXX_ARCH_PPC
# endif
#elif defined(__sparc)
# define DSPXX_ARCH_SPARC
#elif defined(__i386) || defined(_M_IX86)
# define DSPXX_ARCH_X86
# define DSPXX_ARCH_FAMILY_X86
# define DSPXX_ENDIAN_LITTLE
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
# define DSPXX_ARCH_X86_64
# define DSPXX_ARCH_FAMILY_X86
# define DSPXX_ENDIAN_LITTLE
#elif defined(__arm__) || defined(_M_ARM)
# define DSPXX_ARCH_ARM
# define DSPXX_ARCH_FAMILY_ARM
#elif defined(__thumb__) || defined(_M_ARMT)
# define DSPXX_ARCH_THUMB
# define DSPXX_ARCH_FAMILY_ARM
#elif defined(__aarch64__)
# define DSPXX_ARCH_ARM64
# define DSPXX_ARCH_FAMILY_ARM
#else
# error "Unsupported processor architecture, tweak dsp++/platform.h to detect it."
#endif

#else // DOXYGEN_RUNNING
//! @defgroup DSPXX_ARCH_XXX Preprocessor macros which unify the way we determine for which platform we're building.
//! @{

//! @brief Defined if we're targeting Itanium (IA-64) platform.
# define DSPXX_ARCH_IA64
//! @brief Defined if we're targeting Power family of processor (PowerPC or 64-bit version thereof).
# define DSPXX_ARCH_FAMILY_PPC
//! @brief Defined if we're targeting 64-bit PowerPC processor.
# define DSPXX_ARCH_PPC64
//! @brief Defined if we're targeting PowerPC processor.
# define DSPXX_ARCH_PPC
//! @brief Defined if we're targeting SPARC processor.
# define DSPXX_ARCH_SPARC

//! @brief Defined if we're targeting x86 family of processors (x86 or x86-64 aka AMD64).
# define DSPXX_ARCH_FAMILY_X86
//! @brief Defined if we're targeting x86 (IA-32) processor.
# define DSPXX_ARCH_X86
//! @brief Defined if we're targeting x86-64 aka AMD64 processor.
# define DSPXX_ARCH_X86_64

//! @}
#endif // DOXYGEN_RUNNING

#ifndef DOXYGEN_RUNNING

#if defined(__ANDROID__)
# define DSPXX_OS_ANDROID
# define DSPXX_ENDIAN_LITTLE
#endif

#if defined(__amigaos__) || defined(AMIGA)
# define DSPXX_OS_AMIGAOS
#endif

#if defined(__BEOS__)
# define DSPXX_OS_BEOS
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
		defined(__bsdi__) || defined(__DragonFly__)
# define DSPXX_OS_FAMILY_BSD
# if defined(__NetBSD__)
#  define DSPXX_OS_NETBSD
# elif defined(__FreeBSD__)
#  define DSPXX_OS_FREEBSD
# elif defined(__OpenBSD__)
#  define DSPXX_OS_OPENBSD
# elif defined(__bsdi__)
#  define DSPXX_OS_BSDOS
# elif defined(__DragonFly__)
#  define DSPXX_OS_DRAGONFLY
# endif
#endif

#if defined(__CYGWIN__)
# define DSPXX_OS_CYGWIN
#endif

#if defined(hpux) || defined(_hpux) || defined(__hpux)
# define DSPXX_OS_HPUX
#endif

#if defined(__OS400__)
# define DSPXX_OS_OS400
#endif

#if defined(__INTERIX)
# define DSPXX_OS_INTERIX
#endif

#if defined(sgi) || defined(__sgi)
# define DSPXX_OS_IRIX
#endif

#if defined(__linux__) || defined(linux) || defined(__linux)
# define DSPXX_OS_LINUX
#endif

#if defined(macintosh) || defined(Macintosh) || defined(__APPLE__)
# define DSPXX_OS_FAMILY_MACOS
# if defined(__MACH__)
#  define DSPXX_OS_MACOSX
# else
#  define DSPXX_OS_MACOS
# endif
#endif

#if defined(__minix)
# define DSPXX_OS_MINIX
#endif

#if defined(MSDOS) || defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
# define DSPXX_OS_MSDOS
#endif

#if defined(OS2) || defined(_OS2) || defined(__OS2__) || defined(__TOS_OS2__)
# define DSPXX_OS_OS2
#endif

#if defined(__QNX__) || defined(__QNXNTO__)
# define DSPXX_OS_QNX
#endif

#if defined(M_I386) || defined(M_XENIX) || defined(_SCO_DS)
# define DSPXX_OS_OPENSERVER
#endif

#if defined(__sysv__) || defined(__SVR4) || defined(__svr4__) || defined(_SYSTYPE_SVR4)
# define DSPXX_OS_FAMILY_SYSV
#endif

#if defined(sun) || defined(__sun)
# define DSPXX_OS_FAMILY_SUN
# if defined(DSPXX_OS_FAMILY_SYSV)
#  define DSPXX_OS_SOLARIS
# else
#  define DSPXX_OS_SUNOS
# endif
#endif

#if defined(__SYMBIAN32__)
# define DSPXX_OS_SYMBIAN
#endif

#if defined(__unix__) || defined(__unix)
# define DSPXX_OS_FAMILY_UNIX
#endif

#if defined(sco) || defined(_UNIXWARE7)
# define DSPXX_OS_UNIXWARE
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || \
	defined(__TOS_WIN__) || defined(__WINDOWS__) || \
	defined(_WIN32_WCE) || defined(__MINGW32__) || defined(__MINGW64__)
# define DSPXX_OS_FAMILY_WINDOWS
# if defined(_WIN16)
#  define DSPXX_OS_WIN16
# elif defined(_WIN32_WCE)
#  define DSPXX_OS_WINCE
#  define DSPXX_OS_FAMILY_WIN32
# elif defined(_WIN64) || defined(__MINGW64__)
#  define DSPXX_OS_WIN64
#  define DSPXX_OS_FAMILY_WIN32
# else
#  define DSPXX_OS_WIN32
#  define DSPXX_OS_FAMILY_WIN32
# endif
#endif

#if defined(__posix__)
# define DSPXX_OS_FAMILY_POSIX
#endif

#else // For Doxygen run define & comment all of the above

//! Android Platform, include <android/api-level.h> and check __ANDROID_API__ for api level
#define DSPXX_OS_ANDROID

//! AmigaOS
#define DSPXX_OS_AMIGAOS

//! BeOS
#define DSPXX_OS_BEOS

//! Defined for all *BSD variations
#define DSPXX_OS_FAMILY_BSD
#define DSPXX_OS_NETBSD
#define DSPXX_OS_FREEBSD
#define DSPXX_OS_OPENBSD
//! BSD/OS
#define DSPXX_OS_BSDOS
#define DSPXX_OS_DRAGONFLY

#define DSPXX_OS_CYGWIN

#define DSPXX_OS_HPUX

//! IBM OS/400
#define DSPXX_OS_OS400

//! Interix aka Microsoft Services for Unix
#define DSPXX_OS_INTERIX

#define DSPXX_OS_IRIX

#define DSPXX_OS_LINUX

//! Defined for both MacOS and MacOS X
#define DSPXX_OS_FAMILY_MACOS
#define DSPXX_OS_MACOSX
#define DSPXX_OS_MACOS

#define DSPXX_OS_MINIX

#define DSPXX_OS_MSDOS

#define DSPXX_OS_OS2

#define DSPXX_OS_QNX

#define DSPXX_OS_OPENSERVER

#define DSPXX_OS_FAMILY_SYSV

#define DSPXX_OS_FAMILY_SUN
#define DSPXX_OS_SOLARIS
#define DSPXX_OS_SUNOS

#define DSPXX_OS_SYMBIAN

#define DSPXX_OS_FAMILY_UNIX

#define DSPXX_OS_UNIXWARE

//! Defined for all Windows-like systems (CE, 16-bit Win 1.0-3.x, 32-bit Windows 9x, 32 & 64 bit Windows NT...
#define DSPXX_OS_FAMILY_WINDOWS
#define DSPXX_OS_WIN16
#define DSPXX_OS_WINCE
//! Defined for all systems using Win32 API (including 64-bit platforms)
#define DSPXX_OS_FAMILY_WIN32
#define DSPXX_OS_WIN64
#define DSPXX_OS_WIN32

#define DSPXX_OS_FAMILY_POSIX

#endif

#ifdef __cplusplus

namespace dsp { namespace byte_order {

enum label {
	little_endian,
	big_endian,
};

#if defined(DSPXX_ENDIAN_LITTLE)
constexpr label platform = little_endian;
#elif defined(DSPXX_ENDIAN_BIG)
constexpr label platform = big_endian;
#else
DSPXX_API extern const label platform;
#endif
}}

#endif /* __cplusplus */
