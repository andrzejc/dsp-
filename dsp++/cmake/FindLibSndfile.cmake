# - Try to find libsndfile
# Once done this will define
#  LIBSNDFILE_FOUND - System has libsndfile
#  LIBSNDFILE_INCLUDE_DIRS - The libsndfile include directories
#  LIBSNDFILE_LIBRARIES - The libraries needed to use libsndfile
#  LIBSNDFILE_DEFINITIONS - Compiler switches required for using libsndfile

if (MSVC)
	set(LIBSNDFILE_ROOT $ENV{LIBSNDFILE_ROOT} CACHE PATH "Installation folder of libsndfile" )
	if (CMAKE_CL_64)
		set(LIBSNDFILE_DIR ${LIBSNDFILE_ROOT}/x64)
	else ()
		set(LIBSNDFILE_DIR ${LIBSNDFILE_ROOT}/x86)
	endif()

#	message("LIBSNDFILE_DIR: ${LIBSNDFILE_DIR}")

	find_library(LIBSNDFILE_LIBRARY NAMES libsndfile libsndfile-1 libsndfile1 sndfile sndfile-1 sndfile1
		HINTS ${LIBSNDFILE_DIR}/lib )

#	message("LIBSNDFILE_LIBRARY: ${LIBSNDFILE_LIBRARY}")

	find_path(LIBSNDFILE_INCLUDE_DIR sndfile.h
		HINTS ${LIBSNDFILE_DIR}/include
		PATH_SUFFIXES sndfile )

#	message("LIBSNDFILE_INCLUDE_DIR: ${LIBSNDFILE_LIBRARY}")

else ()
	find_package(PkgConfig)
	pkg_check_modules(PC_LIBSNDFILE QUIET sndfile)
	set(LIBSNDFILE_DEFINITIONS ${PC_LIBSNDFILE_CFLAGS_OTHER})

	find_path(LIBSNDFILE_INCLUDE_DIR sndfile.h
          HINTS ${PC_LIBSNDFILE_INCLUDEDIR} ${PC_LIBSNDFILE_INCLUDE_DIRS}
          PATH_SUFFIXES sndfile )

	find_library(LIBSNDFILE_LIBRARY NAMES libsndfile sndfile
             HINTS ${PC_LIBSNDFILE_LIBDIR} ${PC_LIBSNDFILE_LIBRARY_DIRS} )

endif()

set(LIBSNDFILE_LIBRARIES ${LIBSNDFILE_LIBRARY} )
set(LIBSNDFILE_INCLUDE_DIRS ${LIBSNDFILE_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LIBSNDFILE_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(LibSndfile  DEFAULT_MSG
                                  LIBSNDFILE_LIBRARIES LIBSNDFILE_INCLUDE_DIRS)

mark_as_advanced(LIBSNDFILE_INCLUDE_DIRS LIBSNDFILE_LIBRARIES )
