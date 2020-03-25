project(dsp++-test)

find_package(GTest REQUIRED CONFIG)

set(SOURCES
    execution_timer.cpp

    test_adaptfilt.cpp
    test_fft.cpp
    test_filter_design.cpp
    test_filter.cpp
    test_intmath.cpp
    test_lattice.cpp
    test_levinson.cpp
    test_lpc.cpp
    test_mean.cpp
    test_overlap_add.cpp
    test_pow2.cpp
    test_resample.cpp
    test_simd.cpp
    test_snd_loudness.cpp
    test_snd_channel.cpp
    test_snd_sample.cpp
    test_sndfile_api.cpp
    test_window.cpp
    test_xcorr.cpp
)

add_executable(dsp++-test ${SOURCES})
target_link_libraries(dsp++-test
    PRIVATE
        dsp++
        GTest::gtest
        GTest::gtest_main
)

install(TARGETS dsp++-test DESTINATION bin)
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:dsp++-test> DESTINATION lib OPTIONAL)
endif()


macro(dsp_test name)
    add_test(NAME "${name}"
        COMMAND dsp++-test "--gtest_filter=${name}.*" --gtest_color=yes
    )
    set_tests_properties("${name}" PROPERTIES
        ENVIRONMENT "DSPXX_TEST_DATA=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/data>"
    )
    if(WIN32)
        string(REPLACE ";" "\\;" escaped_path "$ENV{PATH}")
        set_property(TEST "${name}" APPEND PROPERTY
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
dsp_test(snd_channel_layout)
dsp_test(snd_sample)
dsp_test(sndfile_api)
