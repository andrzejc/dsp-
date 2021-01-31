project(filter_demo CXX)

add_executable(filter_demo_freq main.cpp)
target_compile_definitions(filter_demo_freq PRIVATE FILTER_FREQ_DOMAIN)
target_link_libraries(filter_demo_freq PRIVATE dsp++)

add_executable(filter_demo_time main.cpp)
target_compile_definitions(filter_demo_time PRIVATE FILTER_TIME_DOMAIN)
target_link_libraries(filter_demo_time PRIVATE dsp++)

if(DSPXX_INSTALL)
    install(TARGETS filter_demo_freq DESTINATION bin)
    install(TARGETS filter_demo_time DESTINATION bin)
endif()
