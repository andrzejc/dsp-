cmake_minimum_required(VERSION 3.11)

project(dsp++
    VERSION 0.1.0
    DESCRIPTION "Digital Signal Processing library written in modern C++"
    HOMEPAGE_URL "https://github.com/andrzejc/dspxx"
    LANGUAGES CXX C
)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}/deps/build-scripts/cmake")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(DSPXX_TOPLEVEL_PROJECT ON)
endif()

option(DSPXX_BUILD_DEMOS "Build example/demo code" OFF)
if(DSPXX_TOPLEVEL_PROJECT)
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
    option(BUILD_TESTING "Build testing code" ON)
    option(DSPXX_INSTALL "Create installable build" ON)
endif()

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
        -D_CRT_NONSTDC_NO_WARNINGS
    )
    # Disable some warnings which make little sense
    add_compile_options(
        /wd4250
        /wd4251
        /wd4275
    )
    # enable parallel compilation
    add_compile_options(/MP)
    # enable exception handling
    add_compile_options(/EHsc)
    # Create pdb files for debugging purposes, also for Release builds
    add_compile_options(/Zi /GS)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG /OPT:ICF /OPT:REF")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:ICF /OPT:REF")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
    # Disable some warnings for Clang, for some things that are too awkward
    # to change just for the sake of having no warnings.
    add_compile_options(-Wall -Wno-unknown-pragmas -Wno-unused-local-typedefs -fdiagnostics-color=always)

    option(ENABLE_ASAN "Enable AddressSanitizer?" OFF)
    if(ENABLE_ASAN)
        add_compile_options(-fno-omit-frame-pointer -fsanitize=address)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fno-omit-frame-pointer -fsanitize=address")
        set(MEMORYCHECK_TYPE AddressSanitizer)
    endif()
endif()

if(ANDROID)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsigned-char")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsigned-char")
endif()

if(BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        set(INSTALL_GTEST OFF CACHE BOOL "")
        add_subdirectory(deps/googletest)
        add_library(GTest::gmock ALIAS gmock)
        add_library(GTest::gmock_main ALIAS gmock_main)
    endif()
    include(CTest)
    enable_testing()
endif()

if(WIN32)
    set(DSPXX_DOC_PREFIX "${PROJECT_NAME}-${PROJECT_VERSION}/doc")
else()
    set(DSPXX_DOC_PREFIX "share/doc/${PROJECT_NAME}-${PROJECT_VERSION}")
endif()
if(DSPXX_INSTALL)
    install(FILES README.md DESTINATION "${DSPXX_DOC_PREFIX}")
    install(FILES LICENSE.txt DESTINATION "${DSPXX_DOC_PREFIX}")
endif()

add_subdirectory(dsp++)

if(DSPXX_BUILD_DEMOS)
    add_subdirectory(compressor_demo)
    add_subdirectory(filter_demo)
endif()

set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VENDOR "Andrzej Ciarkowski")
set(CPACK_PACKAGE_CONTACT "andrzej.ciarkowski@gmail.com")

if(WIN32)
    set(CPACK_GENERATOR ZIP)
endif()
include(CPack)
