project(dsp++-test)

message(STATUS "Building embedded GTest dependency")
set(INSTALL_GTEST OFF CACHE BOOL "We don't want to install GTest as a part of dsp++ installation")
set(gtest_force_shared_crt ON CACHE BOOL "Use of static crt causes multiply-defined symbols on MSVC")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(../../deps/googletest "${CMAKE_CURRENT_BINARY_DIR}/googletest")

set(SOURCES
    execution_timer.cpp

    adaptfilt_test.cpp
    fft_test.cpp
    filter_design_test.cpp
    filter_test.cpp
    intmath_test.cpp
    lattice_test.cpp
    levinson_test.cpp
    loudness_test.cpp
    lpc_test.cpp
    mean_test.cpp
    overlap_add_test.cpp
    pow2_test.cpp
    resample_test.cpp
    simd_test.cpp
    window_test.cpp
    xcorr_test.cpp
)

add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test
    PRIVATE
        dsp++
        gtest
        gtest_main
)

install(TARGETS dsp++-test DESTINATION bin)
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:dsp++-test> DESTINATION lib OPTIONAL)
endif()


macro(dsp_test name)
    add_test(NAME "${name}"
        COMMAND dsp++-test "--gtest_filter=${name}.*" --gtest_color=yes
    )
    if(WIN32)
        string(REPLACE ";" "\\;" escaped_path "$ENV{PATH}")
        set_tests_properties("${name}" PROPERTIES
            ENVIRONMENT "PATH=${escaped_path}\\;\
$<SHELL_PATH:$<TARGET_LINKER_FILE_DIR:dsp++>>\\;\
$<SHELL_PATH:$<TARGET_PROPERTY:Sndfile::libsndfile,DLL_DIRECTORY>>\\;\
$<SHELL_PATH:$<TARGET_PROPERTY:FFTW3::libfftw3,DLL_DIRECTORY>>"
        )
    endif()
endmacro()

dsp_test(adaptfilt)
dsp_test(fft)
dsp_test(filter_design)
dsp_test(filter)
dsp_test(intmath)
dsp_test(lattice)
dsp_test(levinson)
# dsp_test(loudness)
dsp_test(lpc)
dsp_test(mean)
# dsp_test(overlap)
dsp_test(pow2)
# dsp_test(resample)
# dsp_test(simd)
dsp_test(window)
dsp_test(xcorr)
