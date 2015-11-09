project(compressor_demo)

set(SOURCES
	main.cpp
)

add_executable(compressor_demo ${SOURCES})
include_directories(${Boost_INCLUDE_DIRS})
add_dependencies(compressor_demo dsp++)
target_link_libraries(compressor_demo dsp++)

install(TARGETS compressor_demo DESTINATION bin)

