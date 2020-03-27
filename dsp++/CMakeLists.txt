project(libdsp++
    ${DSPXX_PROJECT_VARS}
    VERSION "${PROJECT_VERSION}"
    LANGUAGES CXX
)

find_package(absl REQUIRED CONFIG)
find_package(Boost REQUIRED)
find_package(Doxygen)
find_package(FFTW3 COMPONENTS libfftw3 libfftw3f libfftw3l)
find_package(Sndfile)
find_package(Mpg123)

# options
option(DSPXX_FFTW3_ENABLED "Enable the use and support for FFTW3 library" ${FFTW3_FOUND})
option(DSPXX_LIBSNDFILE_ENABLED "Enable the use and support for libsndfile library" ${Sndfile_FOUND})
option(DSPXX_MPG123_ENABLED "Enable the use and support for libmpg123 mp3 decoding?" ${Mpg123_FOUND})
option(DSPXX_BUILD_DOCS "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

set(SOURCES
    src/csv.cpp
    src/debug.cpp
    src/fft.cpp
    src/filter.cpp
    src/flt_biquad.cpp
    src/flt_fs.cpp
    src/flt_iir.cpp
    src/flt_pm.cpp
    src/resample.cpp
    src/simd.cpp
    src/vectmath.cpp
    src/zeropole.cpp
    src/version.cpp
    src/fftw/traits.cpp
    src/mkfilter/mkfilter.cpp
    src/remez/remez.cpp
    src/rpoly/rpoly.cpp
    src/snd/channel.cpp
    src/snd/format.cpp
    src/snd/loudness.cpp
    src/snd/mpg123_api.cpp
    src/snd/sample.cpp
    src/snd/sndfile_api.cpp
)


# platform-specific stuff
include(TargetPlatform)
# x86 - specific sources & configuration
if(TARGET_PLATFORM_X86)
    list(APPEND SOURCES
        src/arch/x86/cpu_x86.cpp
        src/arch/x86/sse.cpp
        src/arch/x86/sse3.cpp
        src/arch/x86/sse41.cpp
        )

    set_source_files_properties(src/arch/x86/sse.cpp PROPERTIES
        COMPILE_FLAGS "${CXX_COMPILER_SSE_FLAG}")
    set_source_files_properties(src/arch/x86/sse3.cpp PROPERTIES
        COMPILE_FLAGS "${CXX_COMPILER_SSE3_FLAG}")
    set_source_files_properties(src/arch/x86/sse41.cpp PROPERTIES
        COMPILE_FLAGS "${CXX_COMPILER_SSE41_FLAG}")

    list(APPEND HEADERS
        src/arch/x86/sse_utils.h
        src/arch/x86/sse.h
        )
endif()
# ARM - specific
if(TARGET_PLATFORM_ARM)
    list(APPEND SOURCES
        src/arch/arm/cpu_arm.cpp
        )
endif()
# PowerPC - specific
if(TARGET_PLATFORM_PPC)
    list(APPEND SOURCES
        src/arch/ppc/cpu_ppc.cpp
        )
endif()

configure_file(src/version.h.in include/dsp++/version.h @ONLY)

find_library(MPG123_LIBRARY NAMES mpg123)

add_library(dsp++ ${SOURCES})
target_include_directories(dsp++
    PRIVATE
        src
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
    )
target_link_libraries(dsp++
    PUBLIC
        Boost::boost
        absl::optional
        absl::strings
    PRIVATE
        absl::hash
        $<$<BOOL:${DSPXX_LIBSNDFILE_ENABLED}>:Sndfile::libsndfile>
        $<$<BOOL:${DSPXX_FFTW3_ENABLED}>:FFTW3::libfftw3>
        $<$<BOOL:${DSPXX_FFTW3_ENABLED}>:FFTW3::libfftw3f>
        $<$<BOOL:${DSPXX_FFTW3_ENABLED}>:FFTW3::libfftw3l>
        $<$<BOOL:${DSPXX_MPG123_ENABLED}>:Mpg123::libmpg123>
)
target_compile_definitions(dsp++
    PRIVATE
        $<$<BOOL:${BUILD_SHARED_LIBS}>:DSPXX_EXPORTS>
        # Turn off min/max macros defined in windows.h
        $<$<BOOL:${WIN32}>:NOMINMAX>
    PUBLIC
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:DSPXX_STATIC>
        $<$<NOT:$<BOOL:${DSPXX_LIBSNDFILE_ENABLED}>>:DSPXX_LIBSNDFILE_DISABLED>
        $<$<NOT:$<BOOL:${DSPXX_FFTW3_ENABLED}>>:DSPXX_FFTW3_DISABLED>
        $<$<NOT:$<BOOL:${DSPXX_MPG123_ENABLED}>>:DSPXX_MPG123_DISABLED>
)
set_target_properties(dsp++ PROPERTIES
    VERSION "${PROJECT_VERSION}"
    CXX_VISIBILITY_PRESET hidden
)

export(TARGETS dsp++ FILE dsp++-exports.cmake)

install(TARGETS dsp++ EXPORT dsp++ DESTINATION lib)
install(DIRECTORY include/dsp++ DESTINATION include)
install(DIRECTORY "${PROJECT_BINARY_DIR}/include/dsp++" DESTINATION include)
if(WIN32)
    install(EXPORT dsp++ DESTINATION "dsp++-${PROJECT_VERSION}/cmake" FILE dsp++-config.cmake)
else()
    install(EXPORT dsp++ DESTINATION "share/dsp++-${PROJECT_VERSION}/cmake" FILE dsp++-config.cmake)
endif()
if(MSVC)
    install(FILES $<TARGET_PDB_FILE:dsp++> DESTINATION lib OPTIONAL)
endif()

if(DSPXX_BUILD_DOCS)
    add_subdirectory(doc)
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()
