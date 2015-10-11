cmake_minimum_required(VERSION 3.0)
project(compressor_demo)

set(SOURCES
	main.cpp
)

if (MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4251 /wd4275")
endif()

find_package(Boost REQUIRED)
add_executable(compressor_demo ${SOURCES})
include_directories(${Boost_INCLUDE_DIRS})
add_dependencies(compressor_demo dsp++)
target_link_libraries(compressor_demo dsp++)
set_property(TARGET compressor_demo PROPERTY CXX_STANDARD 11)

install(TARGETS compressor_demo DESTINATION bin)

