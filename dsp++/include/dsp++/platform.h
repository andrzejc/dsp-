/**
 * @file dsp++/platform.h
 * Preprocessor macros which unify the way we determine for which platform we're building.
 */

#ifndef DSP_PLATFORM_H_INCLUDED
#define DSP_PLATFORM_H_INCLUDED
#pragma once

// All the tested macros are listed on http://nadeausoftware.com/articles/2012/02/c_c_tip_how_detect_processor_type_using_compiler_predefined_macros
#if defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
# define DSP_ARCH_IA64
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
#elif defined(__x86_64__) || defined(_M_X64) || defined(_M_AMD64)
# define DSP_ARCH_X86_64
# define DSP_ARCH_FAMILY_X86
#else
// TODO detect and classify ARM systems in platform.h
# error "Unsupported processor architecture, tweak dsp++/platform.h to detect it."
#endif

#endif /* DSP_PLATFORM_H_INCLUDED */
