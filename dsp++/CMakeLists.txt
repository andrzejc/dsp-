project(libdsp++)
set(DSPXX_VERSION_MAJOR 0)
set(DSPXX_VERSION_MINOR 1)
set(DSPXX_VERSION "${DSPXX_VERSION_MAJOR}.${DSPXX_VERSION_MINOR}")

find_package(Doxygen)
# options
option(FFTW3_DISABLED "Disable the use and support for FFTW3 library" OFF)
option(LIBSNDFILE_DISABLED "Disable the use and support for libsndfile library" OFF)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# consider detecting fftw and setting FFTW_DISABLED automatically
if (NOT FFTW3_DISABLED)
	find_package(FFTW3 REQUIRED)
	list(APPEND INCLUDE_DIRS ${FFTW3_INCLUDE_DIRS})
	list(APPEND LIBS ${FFTW3_LIBRARIES})
else ()
	add_definitions(-DDSP_FFTW_DISABLED=1)
endif ()

# consider detecting libsndfile and setting LIBSNDFILE_DISABLED automatically
if (NOT LIBSNDFILE_DISABLED)
	find_package(LibSndfile REQUIRED)
	list(APPEND INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIRS})
	list(APPEND LIBS ${LIBSNDFILE_LIBRARIES})
else ()
	add_definitions(-DDSP_SNDFILE_DISABLED=1)
endif ()

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
)

# platform-specific stuff
include(TargetPlatform)
# x86 - specific sources & configuration
if (TARGET_PLATFORM_X86)
	list(APPEND SOURCES
		src/arch/x86/cpu_x86.cpp
		src/arch/x86/sse.cpp
		src/arch/x86/sse3.cpp
		src/arch/x86/sse41.cpp
	)

	set_source_files_properties(src/arch/x86/sse.cpp PROPERTIES COMPILE_FLAGS "${CXX_COMPILER_SSE_FLAG}")
	set_source_files_properties(src/arch/x86/sse3.cpp PROPERTIES COMPILE_FLAGS "${CXX_COMPILER_SSE3_FLAG}")
	set_source_files_properties(src/arch/x86/sse41.cpp PROPERTIES COMPILE_FLAGS "${CXX_COMPILER_SSE41_FLAG}")
	list(APPEND HEADERS
		src/arch/x86/sse_utils.h
		src/arch/x86/sse.h
	)
endif()
# ARM - specific
if (TARGET_PLATFORM_ARM)
	list(APPEND SOURCES
		src/arch/arm/cpu_arm.cpp
	)
endif()
# PowerPC - specific
if (TARGET_PLATFORM_PPC)
	list(APPEND SOURCES
		src/arch/ppc/cpu_ppc.cpp
	)
endif()
#

if (MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4251 /wd4275")
endif()

find_package(Boost REQUIRED)
list(APPEND INCLUDE_DIRS ${Boost_INCLUDE_DIRS})

add_library(dsp++ SHARED ${SOURCES})
include_directories(src include ${PROJECT_BINARY_DIR}/include ${INCLUDE_DIRS})
target_compile_definitions(dsp++ PRIVATE DSPXX_EXPORTS)
target_include_directories(dsp++ INTERFACE include)
target_include_directories(dsp++ PRIVATE src)
target_link_libraries(dsp++ ${LIBS})
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)
set_target_properties(dsp++ PROPERTIES VERSION ${DSPXX_VERSION})

install(TARGETS dsp++ DESTINATION lib)
install(DIRECTORY include/dsp++/ DESTINATION include/dsp++)
install(FILES ${PROJECT_BINARY_DIR}/include/dsp++/version.h DESTINATION include/dsp++)

if (BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()

	configure_file(doxyfile.in doxyfile @ONLY)

	add_custom_target(doc
	COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen"
	VERBATIM)

	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/ DESTINATION share/doc/${CMAKE_PROJECT_NAME}-${DSPXX_VERSION})
endif()

add_subdirectory(test)
