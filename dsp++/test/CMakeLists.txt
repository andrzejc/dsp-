cmake_minimum_required(VERSION 2.8)
project(dsp++-test)

set(SOURCES
	execution_timer.h
	execution_timer.cpp
	utils.h

	adaptfilt_test.cpp
	fft_test.cpp
	filter_design_test.cpp
	filter_test.cpp
	main.cpp
)

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test dsp++)
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)
install(TARGETS dsp++-test DESTINATION bin)

enable_testing()

add_test(adaptfilt dsp++-test --run-test=adaptfilt)
add_test(fft dsp++-test --run-test=fft)
add_test(filter_design dsp++-test --run-test=filter_design)
add_test(filter dsp++-test --run-test=filter)

