/*!
* @file dsp++/compat/enum_class.h
* @brief Compiler support for enum class/strongly typed enums
* @author Andrzej Ciarkowski <mailto:andrzej.ciarkowski@gmail.com>
*/

#ifndef DSP_COMPAT_ENUM_CLASS_H_INCLUDED
#define DSP_COMPAT_ENUM_CLASS_H_INCLUDED
#pragma once

#include <dsp++/config.h>

#if (defined(DSP_GCC_VERSION) && (DSP_GCC_VERSION > 404))
#define DSP_CXX_SUPPORT_ENUM_CLASS
#elif (defined(_MSC_VER) && (_MSC_VER >= 1700))
#define DSP_CXX_SUPPORT_ENUM_CLASS
#endif

#ifdef DSP_CXX_SUPPORT_ENUM_CLASS
#define enum_class(name) enum class name 
#define enum_class_end	
#define enum_class_ref(name) name
#else
#define enum_class(name) namespace name { enum spec 
#define enum_class_end }
#define enum_class_ref(name) name::spec
#endif

#endif // DSP_COMPAT_ENUM_CLASS_H_INCLUDED
