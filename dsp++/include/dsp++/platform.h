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

#endif /* DSP_PLATFORM_H_INCLUDED */
