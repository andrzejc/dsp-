project(dsp++-test)

set(SOURCES
	execution_timer.h
	execution_timer.cpp
	utils.h

	adaptfilt_test.cpp
	fft_test.cpp
	filter_design_test.cpp
	filter_test.cpp
	intmath_test.cpp
	lattice_test.cpp
	levinson_test.cpp
	loudness_test.cpp
	lpc_test.cpp
	mean_test.cpp
	overlap_add_test.cpp
	pow2_test.cpp
	resample_test.cpp
#	simd_test.cpp
	window_test.cpp
	xcorr_test.cpp

	main.cpp
)

if (MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4251 /wd4275")
endif()

add_definitions(-DBOOST_TEST_DYN_LINK)
include_directories(${Boost_INCLUDE_DIRS})
add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test dsp++ ${Boost_LIBRARIES})
set_property(TARGET dsp++-test PROPERTY CXX_STANDARD 11)
install(TARGETS dsp++-test DESTINATION bin)

macro(dsp_test name)
	add_test(${name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dsp++-test --run_test=${name})
endmacro()

dsp_test(adaptfilt)
dsp_test(fft)
dsp_test(filter_design)
dsp_test(filter)
dsp_test(intmath)
dsp_test(lattice)
dsp_test(levinson)
# dsp_test(loudness)
dsp_test(lpc)
dsp_test(mean)
# dsp_test(overlap)
dsp_test(pow2)
# dsp_test(resample)
# dsp_test(simd)
dsp_test(window)
dsp_test(xcorr)

