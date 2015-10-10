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

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

find_package(Boost REQUIRED COMPONENTS unit_test_framework)
add_definitions(-DBOOST_TEST_DYN_LINK) 
include_directories(${Boost_INCLUDE_DIRS} ${HEADERS_EXTRA})
add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test dsp++ ${Boost_LIBRARIES})
set_property(TARGET dsp++-test PROPERTY CXX_STANDARD 11)
install(TARGETS dsp++-test DESTINATION bin)

add_test(adaptfilt dsp++-test --run_test=adaptfilt)
add_test(fft dsp++-test --run_test=fft)
add_test(filter_design dsp++-test --run_test=filter_design)
add_test(filter dsp++-test --run_test=filter)
add_test(intmath dsp++-test --run_test=intmath)
add_test(lattice dsp++-test --run_test=lattice)
add_test(levinson dsp++-test --run_test=levinson)
# add_test(loudness dsp++-test --run_test=loudness)
add_test(lpc dsp++-test --run_test=lpc)
add_test(mean dsp++-test --run_test=mean)
# add_test(overlap_add dsp++-test --run_test=overlap_add)
add_test(pow2 dsp++-test --run_test=pow2)
# add_test(resample dsp++-test --run_test=resample)
# add_test(simd dsp++-test --run_test=simd)
add_test(window dsp++-test --run_test=window)
add_test(xcorr dsp++-test --run_test=xcorr)

