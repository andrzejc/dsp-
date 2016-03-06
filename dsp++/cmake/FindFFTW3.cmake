# - Try to find FFTW3 libraries
# Once done this will define
#  FFTW3_FOUND - System has libsndfile
#  FFTW3_INCLUDE_DIRS - The libsndfile include directories
#  FFTW3_LIBRARIES - The libraries needed to use libsndfile
#  FFTW3_DEFINITIONS - Compiler switches required for using libsndfile

if (MSVC)
	set(FFTW3_ROOT $ENV{FFTW3_ROOT} CACHE PATH "Installation folder of FFTW3" )
	set(FFTW3_VERSION_MINOR "3" CACHE STRING "Minor version number of FFTW3 libraries" )

	if (CMAKE_CL_64)
		set(FFTW3_DIR ${FFTW3_ROOT}/x64)
	else ()
		set(FFTW3_DIR ${FFTW3_ROOT}/x86)
	endif()

	find_library(FFTW3D_LIBRARY NAMES "libfftw3-${FFTW3_VERSION_MINOR}"
		PATHS ${FFTW3_DIR} ${FFTW3_ROOT} ${FFTW3_ROOT}/lib )

	find_library(FFTW3F_LIBRARY NAMES "libfftw3f-${FFTW3_VERSION_MINOR}"
		PATHS ${FFTW3_DIR} ${FFTW3_ROOT} ${FFTW3_ROOT}/lib )

	find_library(FFTW3L_LIBRARY NAMES "libfftw3l-${FFTW3_VERSION_MINOR}"
		PATHS ${FFTW3_DIR} ${FFTW3_ROOT} ${FFTW3_ROOT}/lib )

	list(APPEND FFTW3_LIBRARY ${FFTW3F_LIBRARY} ${FFTW3D_LIBRARY} ${FFTW3L_LIBRARY} )

	find_path(FFTW3_INCLUDE_DIR fftw3.h
		PATHS ${FFTW3_DIR} ${FFTW3_ROOT}
		PATH_SUFFIXES include fftw3 include/fftw3 )

else ()
	find_package(PkgConfig)
	pkg_check_modules(PC_FFTW3F QUIET fftw3f)

	find_library(FFTW3F_LIBRARY
		NAMES fftw3f libfftw3f
		HINTS ${PC_FFTW3F_LIBDIR} ${PC_FFTW3F_LIBRARY_DIRS} )
	find_library(FFTW3F_THREADS_LIBRARY
		NAMES fftw3f_threads libfftw3f_threads
		HINTS ${PC_FFTW3F_LIBDIR} ${PC_FFTW3F_LIBRARY_DIRS} )
	if(FFTW3F_THREADS_LIBRARY)
		list(APPEND FFTW3F_LIBRARY ${FFTW3F_THREADS_LIBRARY} )
	endif()
	set(FFTW3F_DEFINITIONS ${PC_FFTW3F_CFLAGS_OTHER} )

	pkg_check_modules(PC_FFTW3D QUIET fftw3)
	find_library(FFTW3D_LIBRARY
		NAMES fftw3 libfftw3
		HINTS ${PC_FFTW3D_LIBDIR} ${PC_FFTW3D_LIBRARY_DIRS} )
	find_library(FFTW3D_THREADS_LIBRARY
		NAMES fftw3_threads libfftw3_threads
		HINTS ${PC_FFTW3D_LIBDIR} ${PC_FFTW3D_LIBRARY_DIRS} )
	if(FFTW3D_THREADS_LIBRARY)
		list(APPEND FFTW3D_LIBRARY ${FFTW3D_THREADS_LIBRARY} )
	endif()
	set(FFTW3D_DEFINITIONS ${PC_FFTW3D_CFLAGS_OTHER} )

	pkg_check_modules(PC_FFTW3L QUIET fftw3l)
	find_library(FFTW3L_LIBRARY
		NAMES fftw3l libfftw3l
		HINTS ${PC_FFTW3L_LIBDIR} ${PC_FFTW3L_LIBRARY_DIRS} )
	find_library(FFTW3L_THREADS_LIBRARY
		NAMES fftw3l_threads libfftw3l_threads
		HINTS ${PC_FFTW3L_LIBDIR} ${PC_FFTW3L_LIBRARY_DIRS} )
	if(FFTW3L_THREADS_LIBRARY)
		list(APPEND FFTW3L_LIBRARY ${FFTW3L_THREADS_LIBRARY} )
	endif()
	set(FFTW3L_DEFINITIONS ${PC_FFTW3L_CFLAGS_OTHER} )

	list(APPEND FFTW3_LIBRARY ${FFTW3F_LIBRARY} ${FFTW3D_LIBRARY} ${FFTW3L_LIBRARY} )

	find_path(FFTW3_INCLUDE_DIR fftw3.h
		HINTS ${PC_FFTW3F_INCLUDEDIR} ${PC_FFTW3F_INCLUDE_DIRS} ${PC_FFTW3D_INCLUDEDIR} ${PC_FFTW3D_INCLUDE_DIRS} ${PC_FFTW3L_INCLUDEDIR} ${PC_FFTW3L_INCLUDE_DIRS}
		PATH_SUFFIXES fftw3 )
endif()

set(FFTW3_LIBRARIES ${FFTW3_LIBRARY} )
set(FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FFTW3_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FFTW3  DEFAULT_MSG
                                  FFTW3_LIBRARIES FFTW3_INCLUDE_DIRS)

mark_as_advanced(FFTW3_INCLUDE_DIRS FFTW3_LIBRARIES )
