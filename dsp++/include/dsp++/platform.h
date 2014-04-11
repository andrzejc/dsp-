/**
 * @file dsp++/platform.h
 * @brief Preprocessor macros which unify the way we determine for which platform we're building.
 * @todo Compile-time detection of ARM targets and their variants.
 */

#ifndef DSP_PLATFORM_H_INCLUDED
#define DSP_PLATFORM_H_INCLUDED
#pragma once

#ifndef DOXYGEN_RUNNING
// All the tested macros are listed on http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
#if defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
# define DSP_ARCH_IA64
# define DSP_ENDIAN_LITTLE
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__)
# define DSP_ARCH_FAMILY_PPC
# if defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || \
	  defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
#  define DSP_ARCH_PPC64
# else
#  define DSP_ARCH_PPC
# endif
#elif defined(__sparc)
# define DSP_ARCH_SPARC
#elif defined(__i386) || defined(_M_IX86)
# define DSP_ARCH_X86
# define DSP_ARCH_FAMILY_X86
# define DSP_ENDIAN_LITTLE
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
# define DSP_ARCH_X86_64
# define DSP_ARCH_FAMILY_X86
# define DSP_ENDIAN_LITTLE
#else
# error "Unsupported processor architecture, tweak dsp++/platform.h to detect it."
#endif

#else // DOXYGEN_RUNNING
//! @defgroup DSP_ARCH_XXX Preprocessor macros which unify the way we determine for which platform we're building.
//! @{

//! @brief Defined if we're targeting Itanium (IA-64) platform.
# define DSP_ARCH_IA64
//! @brief Defined if we're targeting Power family of processor (PowerPC or 64-bit version thereof).
# define DSP_ARCH_FAMILY_PPC
//! @brief Defined if we're targeting 64-bit PowerPC processor.
# define DSP_ARCH_PPC64
//! @brief Defined if we're targeting PowerPC processor.
# define DSP_ARCH_PPC
//! @brief Defined if we're targeting SPARC processor.
# define DSP_ARCH_SPARC

//! @brief Defined if we're targeting x86 family of processors (x86 or x86-64 aka AMD64).
# define DSP_ARCH_FAMILY_X86
//! @brief Defined if we're targeting x86 (IA-32) processor.
# define DSP_ARCH_X86
//! @brief Defined if we're targeting x86-64 aka AMD64 processor.
# define DSP_ARCH_X86_64

//! @}
#endif // DOXYGEN_RUNNING

#ifndef DOXYGEN_RUNNING

#if defined(__ANDROID__)
# define DSP_OS_ANDROID
#endif

#if defined(__amigaos__) || defined(AMIGA)
# define DSP_OS_AMIGAOS
#endif

#if defined(__BEOS__)
# define DSP_OS_BEOS
#endif

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
		defined(__bsdi__) || defined(__DragonFly__)
# define DSP_OS_FAMILY_BSD
# if defined(__NetBSD__)
#  define DSP_OS_NETBSD
# elif defined(__FreeBSD__)
#  define DSP_OS_FREEBSD
# elif defined(__OpenBSD__)
#  define DSP_OS_OPENBSD
# elif defined(__bsdi__)
#  define DSP_OS_BSDOS
# elif defined(__DragonFly__)
#  define DSP_OS_DRAGONFLY
# endif
#endif

#if defined(__CYGWIN__)
# define DSP_OS_CYGWIN
#endif

#if defined(hpux) || defined(_hpux) || defined(__hpux)
# define DSP_OS_HPUX
#endif

#if defined(__OS400__)
# define DSP_OS_OS400
#endif

#if defined(__INTERIX)
# define DSP_OS_INTERIX
#endif

#if defined(sgi) || defined(__sgi)
# define DSP_OS_IRIX
#endif

#if defined(__linux__) || defined(linux) || defined(__linux)
# define DSP_OS_LINUX
#endif

#if defined(macintosh) || defined(Macintosh) || defined(__APPLE__)
# define DSP_OS_FAMILY_MACOS
# if defined(__MACH__)
#  define DSP_OS_MACOSX
# else
#  define DSP_OS_MACOS
# endif
#endif

#if defined(__minix)
# define DSP_OS_MINIX
#endif

#if defined(MSDOS) || defined(__MSDOS__) || defined(_MSDOS) || defined(__DOS__)
# define DSP_OS_MSDOS
#endif

#if defined(OS2) || defined(_OS2) || defined(__OS2__) || defined(__TOS_OS2__)
# define DSP_OS_OS2
#endif

#if defined(__QNX__) || defined(__QNXNTO__)
# define DSP_OS_QNX
#endif

#if defined(M_I386) || defined(M_XENIX) || defined(_SCO_DS)
# define DSP_OS_OPENSERVER
#endif

#if defined(__sysv__) || defined(__SVR4) || defined(__svr4__) || defined(_SYSTYPE_SVR4)
# define DSP_OS_FAMILY_SYSV
#endif

#if defined(sun) || defined(__sun)
# define DSP_OS_FAMILY_SUN
# if defined(DSP_OS_FAMILY_SYSV)
#  define DSP_OS_SOLARIS
# else
#  define DSP_OS_SUNOS
# endif
#endif

#if defined(__SYMBIAN32__)
# define DSP_OS_SYMBIAN
#endif

#if defined(__unix__) || defined(__unix)
# define DSP_OS_FAMILY_UNIX
#endif

#if defined(sco) || defined(_UNIXWARE7)
# define DSP_OS_UNIXWARE
#endif

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || \
	defined(__TOS_WIN__) || defined(__WINDOWS__) || \
	defined(_WIN32_WCE) || defined(__MINGW32__) || defined(__MINGW64__)
# define DSP_OS_FAMILY_WINDOWS
# if defined(_WIN16)
#  define DSP_OS_WIN16
# elif defined(_WIN32_WCE)
#  define DSP_OS_WINCE
#  define DSP_OS_FAMILY_WIN32
# elif defined(_WIN64) || defined(__MINGW64__)
#  define DSP_OS_WIN64
#  define DSP_OS_FAMILY_WIN32
# else
#  define DSP_OS_WIN32
#  define DSP_OS_FAMILY_WIN32
# endif
#endif

#if defined(__posix__)
# define DSP_OS_FAMILY_POSIX
#endif

#else // For Doxygen run define & comment all of the above

//! Android Platform, include <android/api-level.h> and check __ANDROID_API__ for api level
#define DSP_OS_ANDROID

//! AmigaOS
#define DSP_OS_AMIGAOS

//! BeOS
#define DSP_OS_BEOS

//! Defined for all *BSD variations
#define DSP_OS_FAMILY_BSD
#define DSP_OS_NETBSD
#define DSP_OS_FREEBSD
#define DSP_OS_OPENBSD
//! BSD/OS
#define DSP_OS_BSDOS
#define DSP_OS_DRAGONFLY

#define DSP_OS_CYGWIN

#define DSP_OS_HPUX

//! IBM OS/400
#define DSP_OS_OS400

//! Interix aka Microsoft Services for Unix
#define DSP_OS_INTERIX

#define DSP_OS_IRIX

#define DSP_OS_LINUX

//! Defined for both MacOS and MacOS X
#define DSP_OS_FAMILY_MACOS
#define DSP_OS_MACOSX
#define DSP_OS_MACOS

#define DSP_OS_MINIX

#define DSP_OS_MSDOS

#define DSP_OS_OS2

#define DSP_OS_QNX

#define DSP_OS_OPENSERVER

#define DSP_OS_FAMILY_SYSV

#define DSP_OS_FAMILY_SUN
#define DSP_OS_SOLARIS
#define DSP_OS_SUNOS

#define DSP_OS_SYMBIAN

#define DSP_OS_FAMILY_UNIX

#define DSP_OS_UNIXWARE

//! Defined for all Windows-like systems (CE, 16-bit Win 1.0-3.x, 32-bit Windows 9x, 32 & 64 bit Windows NT...
#define DSP_OS_FAMILY_WINDOWS
#define DSP_OS_WIN16
#define DSP_OS_WINCE
//! Defined for all systems using Win32 API (including 64-bit platforms)
#define DSP_OS_FAMILY_WIN32
#define DSP_OS_WIN64
#define DSP_OS_WIN32

#define DSP_OS_FAMILY_POSIX

#endif

#endif /* DSP_PLATFORM_H_INCLUDED */
