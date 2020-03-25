project(filter_demo)

add_executable(filter_demo_freq main.cpp)
target_compile_definitions(filter_demo_freq PRIVATE FILTER_FREQ_DOMAIN)
target_link_libraries(filter_demo_freq PRIVATE dsp++)
install(TARGETS filter_demo_freq DESTINATION bin)

add_executable(filter_demo_time main.cpp)
target_compile_definitions(filter_demo_time PRIVATE FILTER_TIME_DOMAIN)
target_link_libraries(filter_demo_time PRIVATE dsp++)
install(TARGETS filter_demo_time DESTINATION bin)
