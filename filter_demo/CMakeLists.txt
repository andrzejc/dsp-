project(filter_demo)

set(SOURCES
	main.cpp
	fdacoefs.h
)

add_executable(filter_demo ${SOURCES})
target_compile_definitions(filter_demo PRIVATE FILTER_FREQ_DOMAIN)
target_link_libraries(filter_demo PRIVATE dsp++)
install(TARGETS filter_demo DESTINATION bin)
