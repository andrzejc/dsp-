if(BUILD_TESTING)
    message(STATUS "Building embedded GTest dependency")
    set(INSTALL_GTEST OFF CACHE BOOL "We don't want to install GTest as a part of dsp++ installation")
    set(gtest_force_shared_crt ON CACHE BOOL "Use of static crt causes multiply-defined symbols on MSVC")
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(googletest)
endif()

# We don't want to run tests of our deps
set(BUILD_TESTING OFF)

if(NOT absl_FOUND)
    message(STATUS "Building embedded abseil dependency")
    add_subdirectory(abseil-cpp)
endif()
