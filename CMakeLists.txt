cmake_minimum_required(VERSION 2.8)

project(dsp++)
cmake_policy(SET CMP0042 NEW)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${PROJECT_BINARY_DIR}/bin CACHE
    PATH "Directory where .exe and .dll files are dumped." FORCE)

enable_testing()

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC" OR CMAKE_CXX_COMPILER_ID MATCHES "Intel")
	# Disable a bunch of stupid warnings from MS SCL/CRT
  	add_definitions(/D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /wd4251 /wd4275)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	# Disable some warnings for Clang, for some things that are too awkward
	# to change just for the sake of having no warnings.
	add_definitions (-Wno-unused-local-typedefs)
#	# Don't warn if we ask it not to warn about warnings it doesn't know
	#	add_definitions (-Wunknown-warning-option)
	add_definitions(-std=gnu++11)
endif()


add_subdirectory(dsp++)
add_subdirectory(compressor_demo)
add_subdirectory(filter_demo)

