project(filter_demo)

add_executable(filter_demo main.cpp)
target_compile_definitions(filter_demo PRIVATE FILTER_FREQ_DOMAIN)
target_link_libraries(filter_demo PRIVATE dsp++)
install(TARGETS filter_demo DESTINATION bin)
