cmake_minimum_required(VERSION 2.8)

project(dsp++)

set(DSPXX_VERSION_MAJOR 0)
set(DSPXX_VERSION_MINOR 1)
set(DSPXX_VERSION_PATCH 0)
set(DSPXX_VERSION ${DSPXX_VERSION_MAJOR}.${DSPXX_VERSION_MINOR}.${DSPXX_VERSION_PATCH})
set(DSPXX_FULL_NAME "${PROJECT_NAME}-${DSPXX_VERSION}")
set(DSPXX_DOC_PREFIX share/doc/${DSPXX_FULL_NAME})
set(DSPXX_SLOGAN "Digital Signal Processing library written in modern C++")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
include(CMakeCompatibility NO_POLICY_SCOPE)

include(BoostOnMSVC)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)

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

install(FILES README.md DESTINATION ${DSPXX_DOC_PREFIX})
install(FILES LICENSE.txt DESTINATION ${DSPXX_DOC_PREFIX})

SET(CPACK_PACKAGE_VERSION_MAJOR ${DSPXX_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${DSPXX_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${DSPXX_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "dsp++ is a ${DSPXX_SLOGAN}")
set(CPACK_PACKAGE_VENDOR "dsp++")
set(CPACK_PACKAGE_CONTACT "andrzej.ciarkowski@gmail.com")
include(CPack)
