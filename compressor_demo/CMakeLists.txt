project(compressor_demo)

set(SOURCES
	main.cpp
)

add_executable(compressor_demo ${SOURCES})
target_link_libraries(compressor_demo PRIVATE ${DSPXX_LIBRARY_TARGET})
install(TARGETS compressor_demo DESTINATION bin)
