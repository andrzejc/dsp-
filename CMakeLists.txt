cmake_minimum_required(VERSION 3.1)
# TODO 3.1 is very fresh version and probably won't be available on most platforms, need to find workaround for setting C++ conformance level to 11

project(dsp++)
cmake_policy(SET CMP0042 NEW)

enable_testing()

add_subdirectory(dsp++)
add_subdirectory(compressor_demo)
add_subdirectory(filter_demo)

