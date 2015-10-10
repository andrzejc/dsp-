cmake_minimum_required(VERSION 3.0)
project(filter_demo)

set(SOURCES
	main.cpp
	fdacoefs.h
)

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

add_executable(filter_demo ${SOURCES})
add_dependencies(filter_demo dsp++)
target_link_libraries(filter_demo dsp++)
set_property(TARGET filter_demo PROPERTY CXX_STANDARD 11)
install(TARGETS filter_demo DESTINATION bin)

