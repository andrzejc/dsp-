cmake_minimum_required(VERSION 2.6)
project(dsp++)
set(DSPXX_VERSION_MAJOR 0)
set(DSPXX_VERSION_MINOR 1)
set(CMAKE_BUILD_TYPE Release)

option(DSP_FFTW_DISABLED "Disable the use and support for FFTW3 library" OFF)
option(DSP_SNDFILE_DISABLED "Disable the use and support for libsndfile library" OFF)

if (NOT DSP_FFTW_DISABLED)
	
	add_library(fftw3 IMPORTED SHARED)
	add_definitions(-DDSP_FFTW_DISABLED=1)
endif (NOT DSP_FFTW_DISABLED)

configure_file(include/dsp++/version.h.in include/dsp++/version.h)

set(SOURCES
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
)

add_library(dsp++ SHARED ${SOURCES})
target_compile_definitions(dsp++ PUBLIC DSPXX_EXPORTS)
target_include_directories(dsp++ INTERFACE include)
target_include_directories(dsp++ PRIVATE src)
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)

install(TARGETS dsp++ DESTINATION lib)
