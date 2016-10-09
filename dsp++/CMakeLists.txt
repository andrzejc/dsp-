project(libdsp++)

find_package(Doxygen)
find_package(FFTW3)
find_package(libsndfile)

# options
option(FFTW3_ENABLED "Enable the use and support for FFTW3 library"
	${FFTW3_FOUND})
option(LIBSNDFILE_ENABLED "Enable the use and support for libsndfile library"
	${libsndfile_FOUND})
option(BUILD_DOCUMENTATION
	"Create and install the HTML based API documentation (requires Doxygen)"
	${DOXYGEN_FOUND})

list(APPEND PUBLIC_HEADERS
	${PROJECT_BINARY_DIR}/include/dsp++/version.h
	include/dsp++/adaptfilt.h
	include/dsp++/algorithm.h
	include/dsp++/buffer.h
	include/dsp++/buffer_traits.h
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
	include/dsp++/fft/detail.h
	include/dsp++/fft.h
	include/dsp++/fftw/allocator.h
	include/dsp++/fftw/dft.h
	include/dsp++/fftw/plan_unavailable.h
	include/dsp++/fftw/traits.h
	include/dsp++/filter.h
	include/dsp++/filter_design.h
	include/dsp++/fixed.h
	include/dsp++/float.h
	include/dsp++/flt/biquad_design.h
	include/dsp++/flt/fir_design.h
	include/dsp++/flt/iir_design.h
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
	include/dsp++/stdint.h
	include/dsp++/stride_iterator.h
	include/dsp++/trivial_array.h
	include/dsp++/utility.h
	include/dsp++/vectmath.h
	include/dsp++/window.h
	include/dsp++/xcorr.h
	include/dsp++/zeropole.h
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
	src/version.cpp
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
if(TARGET_PLATFORM_X86)
	list(APPEND SOURCES
		src/arch/x86/cpu_x86.cpp
		src/arch/x86/sse.cpp
		src/arch/x86/sse3.cpp
		src/arch/x86/sse41.cpp
		)

	set_source_files_properties(src/arch/x86/sse.cpp PROPERTIES
		COMPILE_FLAGS "${CXX_COMPILER_SSE_FLAG}")
	set_source_files_properties(src/arch/x86/sse3.cpp PROPERTIES
		COMPILE_FLAGS "${CXX_COMPILER_SSE3_FLAG}")
	set_source_files_properties(src/arch/x86/sse41.cpp PROPERTIES
		COMPILE_FLAGS "${CXX_COMPILER_SSE41_FLAG}")

	list(APPEND HEADERS
		src/arch/x86/sse_utils.h
		src/arch/x86/sse.h
		)
endif()
# ARM - specific
if(TARGET_PLATFORM_ARM)
	list(APPEND SOURCES
		src/arch/arm/cpu_arm.cpp
		)
endif()
# PowerPC - specific
if(TARGET_PLATFORM_PPC)
	list(APPEND SOURCES
		src/arch/ppc/cpu_ppc.cpp
		)
endif()
#

configure_file(src/version.h.in include/dsp++/version.h @ONLY)

if(BUILD_SHARED)
	add_library(dsp++ SHARED ${SOURCES})
	target_compile_definitions(dsp++ PRIVATE DSPXX_EXPORTS)
	target_include_directories(dsp++ PRIVATE src)
	target_include_directories(dsp++ PUBLIC
		$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
	target_link_libraries(dsp++ PUBLIC Boost::boost)

	if(LIBSNDFILE_ENABLED)
		target_link_libraries(dsp++ PRIVATE libsndfile)
	else()
		target_compile_definitions(dsp++ PRIVATE DSPXX_LIBSNDFILE_DISABLED)
	endif()

	if(FFTW3_ENABLED)
		target_link_libraries(dsp++ PRIVATE fftw3 fftw3f fftw3l)
	else()
		target_compile_definitions(dsp++ PRIVATE DSPXX_FFTW3_DISABLED)
	endif()

	set_target_properties(dsp++ PROPERTIES
		VERSION ${DSPXX_VERSION})

	export(TARGETS dsp++ FILE dsp++-exports.cmake)

	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:dsp++> DESTINATION lib OPTIONAL)
		include(CopyDlls)
		# TODO: modify copy_dlls to work on targets not only files
		copy_dlls(dsp++ ${LIBS})
	endif()

	install(TARGETS dsp++ EXPORT dsp++ DESTINATION lib)
	install(DIRECTORY include/dsp++ DESTINATION include)
	install(DIRECTORY ${PROJECT_BINARY_DIR}/include/dsp++ DESTINATION include)
	install(EXPORT dsp++ DESTINATION lib/dsp++ FILE dsp++-config.cmake)
endif()

if(BUILD_STATIC)
 	add_library(dsp++-static STATIC ${SOURCES})
	target_compile_definitions(dsp++-static PRIVATE DSPXX_STATIC)
	target_include_directories(dsp++-static PRIVATE src)
	target_include_directories(dsp++-static PUBLIC
			$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
			$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
			$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>)
	target_link_libraries(dsp++-static PUBLIC Boost::boost)

	if(LIBSNDFILE_ENABLED)
		target_link_libraries(dsp++-static PUBLIC libsndfile)
	else()
		target_compile_definitions(dsp++-static PRIVATE DSPXX_LIBSNDFILE_DISABLED)
	endif()

	if(FFTW3_ENABLED)
		target_link_libraries(dsp++-static PUBLIC fftw3 fftw3f fftw3l)
	else()
		target_compile_definitions(dsp++-static PRIVATE DSPXX_FFTW3_DISABLED)
	endif()

	set_target_properties(dsp++-static PROPERTIES
		VERSION ${DSPXX_VERSION}
		CXX_VISIBILITY_PRESET default
		POSITION_INDEPENDENT_CODE ON)

	export(TARGETS dsp++-static FILE dsp++-static-exports.cmake)

	if(MSVC)
		install(FILES $<TARGET_PDB_FILE:dsp++-static> DESTINATION lib OPTIONAL)
		copy_dlls(dsp++-static ${LIBS})
	endif()

	install(TARGETS dsp++-static EXPORT dsp++-static DESTINATION lib)
	install(EXPORT dsp++-static DESTINATION lib/dsp++-static FILE dsp++-static-config.cmake)
endif()

if(BUILD_DOCUMENTATION)
	if(NOT DOXYGEN_FOUND)
		message(FATAL_ERROR "Doxygen is needed to build the documentation.")
	endif()

	configure_file(doxyfile.in doxyfile @ONLY)

	add_custom_command(OUTPUT doc/html/toc.xml
		COMMAND ${DOXYGEN_EXECUTABLE} doxyfile
		MAIN_DEPENDENCY "${CMAKE_CURRENT_BINARY_DIR}/doxyfile"
		DEPENDS ${SOURCES}
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM)

	add_custom_target(dsp++-doc ALL
		DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/doc/html/toc.xml")

	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
		DESTINATION ${DSPXX_DOC_PREFIX}
		OPTIONAL)
endif()

if(BUILD_TESTING)
	add_subdirectory(test)
endif()
