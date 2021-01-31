project(compressor_demo CXX)

add_executable(compressor_demo main.cpp)
target_link_libraries(compressor_demo PRIVATE dsp++)
if(DSPXX_INSTALL)
    install(TARGETS compressor_demo DESTINATION bin)
endif()
