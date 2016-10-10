cmake_minimum_required(VERSION 2.8)
project(dsp++)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/build-scripts/cmake)
include(CMakeCompatibility NO_POLICY_SCOPE)

set(DSPXX_VERSION 0.1.0)
include(SetupVersion)
setup_version(DSPXX)

set(DSPXX_FULL_NAME "${PROJECT_NAME}-${DSPXX_VERSION}")
set(DSPXX_DOC_PREFIX share/doc/${DSPXX_FULL_NAME})
set(DSPXX_SLOGAN "Digital Signal Processing library written in modern C++")

option(BUILD_SHARED "Build shared libraries" ON)
option(BUILD_STATIC "Build static libraries" ON)

if(NOT BUILD_STATIC AND NOT BUILD_SHARED)
	message(FATAL_ERROR "At least one of options BUILD_SHARED and BUILD_STATIC must be set")
elseif(BUILD_SHARED)
	set(DSPXX_LIBRARY_TARGET dsp++)
	set(Boost_USE_STATIC_LIBS OFF)
else()
	set(DSPXX_LIBRARY_TARGET dsp++-static)
	set(Boost_USE_STATIC_LIBS ON)
	if(CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc -static-libstdc++")
	endif()
endif()

include(BoostOnMSVC)
find_package(Boost REQUIRED OPTIONAL_COMPONENTS unit_test_framework)

option(BUILD_TESTING "Build testing code" ${Boost_UNIT_TEST_FRAMEWORK_FOUND})
option(BUILD_DEMOS "Build example/demo code" OFF)

if(MSVC OR (WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "Intel"))
	add_definitions(
		/D_CRT_SECURE_NO_WARNINGS # no warnings on perfectly fine C++ code
		/D_SCL_SECURE_NO_WARNINGS
		/wd4251
		/wd4275
		/MP # enable parallel  build too
		/EHsc # enable exception handling
		)
endif()

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	add_definitions(-std=c++11)
elseif(COMPILER_SUPPORTS_CXX0X)
	add_definitions(-std=c++0x)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
	# Disable some warnings for Clang, for some things that are too awkward
	# to change just for the sake of having no warnings.
	add_definitions(-Wno-unused-local-typedefs)
endif()

if(BUILD_TESTING)
	enable_testing()
endif()

add_subdirectory(dsp++)

if(BUILD_DEMOS)
	add_subdirectory(compressor_demo)
	add_subdirectory(filter_demo)
endif()

install(FILES README.md DESTINATION ${DSPXX_DOC_PREFIX})
install(FILES LICENSE.txt DESTINATION ${DSPXX_DOC_PREFIX})

set(CPACK_PACKAGE_VERSION_MAJOR ${DSPXX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${DSPXX_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${DSPXX_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "dsp++ is a ${DSPXX_SLOGAN}")
set(CPACK_PACKAGE_VENDOR "Andrzej Ciarkowski")
set(CPACK_PACKAGE_CONTACT "andrzej.ciarkowski@gmail.com")

include(CPack)
