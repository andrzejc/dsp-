find_package(Doxygen REQUIRED)

set(DOXYGEN_PROJECT_NAME dsp++)
set(DOXYGEN_STRIP_FROM_INC_PATH
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}/include"
)
set(DOXYGEN_STRIP_FROM_PATH
    ${DOXYGEN_STRIP_FROM_INC_PATH}
    "${PROJECT_SOURCE_DIR}/src"
)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
set(DOXYGEN_EXTRACT_LOCAL_CLASSES NO)
set(DOXYGEN_SORT_BY_SCOPE_NAME YES)
set(DOXYGEN_EXCLUDE_PATTERNS *.in)
set(DOXYGEN_EXCLUDE
    pch.h
    remez/remez.h
    remez/remez.cpp
    mkfilter/mkfilter.h
    mkfilter/mkfilter.cpp
)
set(DOXYGEN_EXCLUDE_SYMBOLS
    detail
    remez
    REMEZ_*
    remez_*
    mkfilter
)
set(DOXYGEN_MACRO_EXPANSION YES)
set(DOXYGEN_EXPAND_ONLY_PREDEF YES)
set(DOXYGEN_PREDEFINED
    "BOOST_CONCEPT_REQUIRES(a, b)=b"
    "DSP_CONCEPT_REQUIRES(a, b)=b"
    "DSPXX_API="
    DOXYGEN_RUNNING
    "DSP_ALIGNED(a)="
)
set(DOXYGEN_EXPAND_AS_DEFINED
    DSP_WINDOW_ITERATOR_SUPPORT
    DSP_SND_CHANNEL_MASK
    DSP_FI_TPARAMS
    DSP_FI_TPARAMS_DECL
    DSP_FI_BIN_TPARAMS
    DSP_FI_BIN_TPARAMS_DECL
)
set(DOXYGEN_GENERATE_TREEVIEW YES)
set(DOXYGEN_USE_MATHJAX YES)

if(DEFINED ENV{CONTINUOUS_INTEGRATION})
    message(STATUS "Attempting to silence Doxygen on CI build...")
    # Silence hundreds of warnings on CI
    set(DOXYGEN_QUIET YES)
    set(DOXYGEN_WARNINGS NO)
endif()

doxygen_add_docs(dsp++-doc
    ${DOXYGEN_STRIP_FROM_PATH}
    COMMENT "Generating API documentation with Doxygen"
)

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/"
    DESTINATION "${DSPXX_DOC_PREFIX}"
    OPTIONAL
)
