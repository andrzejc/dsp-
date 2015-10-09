cmake_minimum_required(VERSION 2.8)
project(dsp++)
set(DSPXX_VERSION_MAJOR 0)
set(DSPXX_VERSION_MINOR 1)
set(CMAKE_BUILD_TYPE Release)
cmake_policy(SET CMP0042 NEW)

# options
option(DSPXX_FFTW_DISABLED "Disable the use and support for FFTW3 library" OFF)
option(DSPXX_LIBSNDFILE_DISABLED "Disable the use and support for libsndfile library" OFF)

# consider detecting fftw and setting DSPXX_FFTW_DISABLED automatically
if (NOT DSPXX_FFTW_DISABLED)
	find_library(FFTW3F_LIB fftw3f)
	find_library(FFTW3F_THREADS_LIB fftw3f_threads)
	find_library(FFTW3_LIB fftw3)
	find_library(FFTW3_THREADS_LIB fftw3_threads)
	find_library(FFTW3L_LIB fftw3l)
	find_library(FFTW3L_THREADS_LIB fftw3l_threads)
	set(LIBS ${LIBS} ${FFTW3F_LIB} ${FFTW3F_THREADS_LIB} ${FFTW3_LIB} ${FFTW3_THREADS_LIB} ${FFTW3L_LIB} ${FFTW3L_THREADS_LIB})
else ()
	add_definitions(-DDSP_FFTW_DISABLED=1)
endif ()

# consider detecting libsndfile and setting DSPXX_LIBSNDFILE_DISABLED automatically
if (NOT DSPXX_LIBSNDFILE_DISABLED)
	find_library(LIBSNDFILE_LIB sndfile)
	set(LIBS ${LIBS} ${LIBSNDFILE_LIB})
else ()
	add_definitions(-DDSP_SNDFILE_DISABLED=1)
endif ()

# platform-specific stuff
include(TargetPlatform.cmake)
# x86 - specific sources & configuration
if (TARGET_PLATFORM_X86)
	set(SOURCES_X86
		src/arch/x86/cpu_x86.cpp
		src/arch/x86/sse.cpp
		src/arch/x86/sse3.cpp
		src/arch/x86/sse41.cpp
	)

	set_source_files_properties(src/arch/x86/sse.cpp PROPERTIES COMPILE_FLAGS ${CXX_COMPILER_SSE_FLAG})
	set_source_files_properties(src/arch/x86/sse3.cpp PROPERTIES COMPILE_FLAGS ${CXX_COMPILER_SSE3_FLAG})
	set_source_files_properties(src/arch/x86/sse41.cpp PROPERTIES COMPILE_FLAGS ${CXX_COMPILER_SSE41_FLAG})
	set(SOURCES_ARCH ${SOURCES_X86})
endif()
# ARM - specific
if (TARGET_PLATFORM_ARM)
	set(SOURCES_ARM
		src/arch/arm/cpu_arm.cpp
	)

	set(SOURCES_ARCH ${SOURCES_ARM})
endif()
# PowerPC - specific
if (TARGET_PLATFORM_PPC)
	set(SOURCES_PPC
		src/arch/ppc/cpu_ppc.cpp
	)

	set(SOURCES_ARCH ${SOURCES_PPC})
endif()
#

# configure_file(include/dsp++/version.h.in include/dsp++/version.h)

set(HEADER_FILES
	include/dsp++/config.h
	include/dsp++/export.h
	include/dsp++/debug.h
)

#
set(SOURCES
	${HEADER_FILES}
	src/debug.cpp
	src/fft.cpp
	src/filter.cpp
	src/fixed.cpp
	src/flt_biquad.cpp
	src/flt_fs.cpp
	src/flt_iir.cpp
	src/flt_pm.cpp
	src/resample.cpp
	src/sample.cpp
	src/simd.cpp
	src/vectmath.cpp
	src/zeropole.cpp
	src/fftw/traits.cpp
	src/mkfilter/mkfilter.cpp
	src/remez/remez.cpp
	src/rpoly/rpoly.cpp
	src/snd/format.cpp
	src/snd/io.cpp
	src/snd/loudness.cpp
	${SOURCES_ARCH}
)

add_library(dsp++ SHARED ${SOURCES})
include_directories(src include)
target_compile_definitions(dsp++ PRIVATE DSPXX_EXPORTS)
target_include_directories(dsp++ INTERFACE include)
target_include_directories(dsp++ PRIVATE src)
target_link_libraries(dsp++ ${LIBS})
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)

install(TARGETS dsp++ DESTINATION lib)
