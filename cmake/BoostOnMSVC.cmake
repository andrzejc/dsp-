if (CMAKE_GENERATOR MATCHES "Visual Studio .*")
    if (CMAKE_CL_64)
        set(BOOST_LIB_PREFIX lib64)
    else ()
        set(BOOST_LIB_PREFIX lib32)
    endif()
    
    if (MSVC60) 
        set(VCVER_SUFFIX "6.0")
    elseif (MSVC70)
        set(VCVER_SUFFIX "7.0")
    elseif (MSVC71)
        set(VCVER_SUFFIX "7.1")
    elseif (MSVC80)
        set(VCVER_SUFFIX "8.0")
    elseif (MSVC90)
        set(VCVER_SUFFIX "9.0")
    elseif (MSVC10)
        set(VCVER_SUFFIX "10.0")
    elseif (MSVC11) 
        set(VCVER_SUFFIX "11.0")
    elseif (MSVC12)
        set(VCVER_SUFFIX "12.0")
    elseif (MSVC14)
        set(VCVER_SUFFIX "14.0")
    endif()
    
    set(BOOST_MSVC_LIBDIR "${BOOST_LIB_PREFIX}-msvc-${VCVER_SUFFIX}")
    if (EXISTS "${BOOST_ROOT}/${BOOST_MSVC_LIBDIR}")
        set(BOOST_LIBRARYDIR "${BOOST_ROOT}/${BOOST_MSVC_LIBDIR}")
    elseif (EXISTS "$ENV{BOOST_ROOT}/${BOOST_MSVC_LIBDIR}")
        set(BOOST_LIBRARYDIR "$ENV{BOOST_ROOT}/${BOOST_MSVC_LIBDIR}")
    endif()
endif()