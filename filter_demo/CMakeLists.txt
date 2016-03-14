project(filter_demo)

set(SOURCES
	main.cpp
	fdacoefs.h
)

add_definitions(-DFILTER_FREQ_DOMAIN)
add_executable(filter_demo ${SOURCES})
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(filter_demo dsp++)
install(TARGETS filter_demo DESTINATION bin)