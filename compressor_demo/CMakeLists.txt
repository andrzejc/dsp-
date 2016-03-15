project(compressor_demo)

set(SOURCES
	main.cpp
)

add_executable(compressor_demo ${SOURCES})
target_include_directories(compressor_demo PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(compressor_demo PRIVATE dsp++)
install(TARGETS compressor_demo DESTINATION bin)