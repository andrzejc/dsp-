cmake_minimum_required(VERSION 3.2)

set(DSPXX_PROJECT_VARS)
if(NOT CMAKE_VERSION VERSION_LESS 3.11)
    set(DSPXX_PROJECT_VARS
        DESCRIPTION "Digital Signal Processing library written in modern C++"
        HOMEPAGE_URL https://github.com/andrzejc/dspxx
    )
endif()

project(dsp++
    VERSION 0.1.0
    ${DSPXX_PROJECT_VARS}
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/deps/build-scripts/cmake")

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTING "Build testing code" ON)
option(DSPXX_BUILD_DEMOS "Build example/demo code" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
    )
    # Disable some warnings which make little sense
    add_compile_options(
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
    add_compile_options(-Wall -Wno-unknown-pragmas -Wno-unused-local-typedefs)
endif()

if(BUILD_TESTING)
    enable_testing()
endif()

if(WIN32)
    set(DSPXX_DOC_PREFIX "${PROJECT_NAME}-${PROJECT_VERSION}/doc")
else()
    set(DSPXX_DOC_PREFIX "share/doc/${PROJECT_NAME}-${PROJECT_VERSION}")
endif()
install(FILES README.md DESTINATION "${DSPXX_DOC_PREFIX}")
install(FILES LICENSE.txt DESTINATION "${DSPXX_DOC_PREFIX}")

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
