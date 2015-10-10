cmake_minimum_required(VERSION 2.8)
project(dsp++)
cmake_policy(SET CMP0042 NEW)

enable_testing()

add_subdirectory(dsp++)
add_subdirectory(compressor_demo)
add_subdirectory(filter_demo)

