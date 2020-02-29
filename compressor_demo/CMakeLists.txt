project(compressor_demo)

add_executable(compressor_demo main.cpp)
target_link_libraries(compressor_demo PRIVATE dsp++)
install(TARGETS compressor_demo DESTINATION bin)
