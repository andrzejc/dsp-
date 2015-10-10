cmake_minimum_required(VERSION 3.0)
project(compressor_demo)

set(SOURCES
	main.cpp
)

if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
	set(HEADERS_EXTRA /usr/local/include)
endif()

add_executable(compressor_demo ${SOURCES})
add_dependencies(compressor_demo dsp++)
target_link_libraries(compressor_demo dsp++)
set_property(TARGET compressor_demo PROPERTY CXX_STANDARD 11)

install(TARGETS compressor_demo DESTINATION bin)

