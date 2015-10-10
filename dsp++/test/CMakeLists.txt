cmake_minimum_required(VERSION 2.8)
project(dsp++-test)

set(SOURCES
	execution_timer.h
	execution_timer.cpp
	utils.h

#	adaptfilt_test.cpp
#	fft_test.cpp
#	filter_design_test.cpp
	filter_test.cpp
	main.cpp
)

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

find_package (Boost REQUIRED COMPONENTS unit_test_framework)
add_definitions(-DBOOST_TEST_DYN_LINK) 
include_directories(${Boost_INCLUDE_DIRS})
add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test dsp++ ${Boost_LIBRARIES})
set_property(TARGET dsp++ PROPERTY CXX_STANDARD 11)
install(TARGETS dsp++-test DESTINATION bin)

add_test(NAME adaptfilt COMMAND dsp++-test --run-test=adaptfilt)
add_test(NAME fft COMMAND dsp++-test --run-test=fft)
add_test(NAME filter_design COMMAND dsp++-test --run-test=filter_design)
add_test(NAME filter COMMAND dsp++-test --run-test=filter)

