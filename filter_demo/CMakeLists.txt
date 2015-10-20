cmake_minimum_required(VERSION 3.0)
project(filter_demo)

set(SOURCES
	main.cpp
	fdacoefs.h
)

if (MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS /wd4251 /wd4275")
endif()

add_executable(filter_demo ${SOURCES})
include_directories(${Boost_INCLUDE_DIRS})
add_dependencies(filter_demo dsp++)
target_link_libraries(filter_demo dsp++)
set_property(TARGET filter_demo PROPERTY CXX_STANDARD 11)
install(TARGETS filter_demo DESTINATION bin)

