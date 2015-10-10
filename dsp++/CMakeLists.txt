cmake_minimum_required(VERSION 3.1)
# TODO 3.1 is very fresh version and probably won't be available on most platforms, need to find workaround for setting C++ conformance level to 11

project(libdsp++)
set(DSPXX_VERSION_MAJOR 0)
set(DSPXX_VERSION_MINOR 1)

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
	set(HEADERS_ARCH
		src/arch/x86/sse_utils.h
		src/arch/x86/sse.h
	)
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

configure_file(src/version.h.in include/dsp++/version.h)

set(PUBLIC_HEADERS
	${PROJECT_BINARY_DIR}/include/dsp++/version.h
	include/dsp++/adaptfilt.h
	include/dsp++/algorithm.h
	include/dsp++/buffer_traits.h
	include/dsp++/buffer.h
	include/dsp++/compat.h
	include/dsp++/complex.h
	include/dsp++/config.h
	include/dsp++/const.h
	include/dsp++/csvread.h
	include/dsp++/debug.h
	include/dsp++/dft.h
	include/dsp++/doc.h
	include/dsp++/dynamics.h
	include/dsp++/export.h
	include/dsp++/fdaf.h
	include/dsp++/fft.h
	include/dsp++/filter_design.h
	include/dsp++/filter.h
	include/dsp++/fixed.h
	include/dsp++/float.h
	include/dsp++/intmath.h
	include/dsp++/ioport.h
	include/dsp++/lattice.h
	include/dsp++/levinson.h
	include/dsp++/lpc.h
	include/dsp++/mean.h
	include/dsp++/noncopyable.h
	include/dsp++/norm.h
	include/dsp++/overlap_add.h
	include/dsp++/overlap_save.h
	include/dsp++/platform.h
	include/dsp++/polyroots.h
	include/dsp++/pow2.h
	include/dsp++/resample.h
	include/dsp++/simd.h
	include/dsp++/stdint.h
	include/dsp++/stride_iterator.h
	include/dsp++/trivial_array.h
	include/dsp++/utility.h
	include/dsp++/vectmath.h
	include/dsp++/window.h
	include/dsp++/xcorr.h
	include/dsp++/zeropole.h
	include/dsp++/compat/enum_class.h
	include/dsp++/fft/detail.h
	include/dsp++/fftw/allocator.h
	include/dsp++/fftw/dft.h
	include/dsp++/fftw/plan_unavailable.h
	include/dsp++/fftw/traits.h
	include/dsp++/flt/biquad_design.h
	include/dsp++/flt/fir_design.h
	include/dsp++/flt/iir_design.h
	include/dsp++/snd/buffer.h
	include/dsp++/snd/convert.h
	include/dsp++/snd/format.h
	include/dsp++/snd/io_error.h
	include/dsp++/snd/iobase.h
	include/dsp++/snd/loudness.h
	include/dsp++/snd/reader.h
	include/dsp++/snd/sample.h
	include/dsp++/snd/sndfile_error.h
	include/dsp++/snd/writer.h
)

set(HEADERS
	${PUBLIC_HEADERS}
	src/polyroots.h
	src/prefix.h
	src/simd.h
	src/utility.h
	src/mkfilter/mkfilter.h
	src/remez/remez.h
	${HEADERS_ARCH}
)

set(SOURCES
	${HEADERS}
	src/csvread.cpp
	src/debug.cpp
	src/fft.cpp
	src/filter.cpp
	src/fixed.cpp
	src/flt_biquad.cpp
	src/flt_fs.cpp
	src/flt_iir.cpp
	src/flt_pm.cpp
	src/format_win32.cpp
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

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

add_library(dsp++ SHARED ${SOURCES})
include_directories(src include ${PROJECT_BINARY_DIR}/include ${HEADERS_EXTRA})
target_compile_definitions(dsp++ PRIVATE DSPXX_EXPORTS)
target_include_directories(dsp++ INTERFACE include)
target_include_directories(dsp++ PRIVATE src)
target_link_libraries(dsp++ ${LIBS})
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)
set_target_properties(dsp++ PROPERTIES VERSION "${DSPXX_VERSION_MAJOR}.${DSPXX_VERSION_MINOR}")

install(TARGETS dsp++ DESTINATION lib)
install(DIRECTORY include/dsp++/ DESTINATION include/dsp++)
install(FILES ${PROJECT_BINARY_DIR}/include/dsp++/version.h DESTINATION include/dsp++)

