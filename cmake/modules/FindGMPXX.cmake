
# Try to find the GMPXX libraries
# GMPXX_FOUND - system has GMPXX lib
# GMPXX_INCLUDE_DIR - the GMPXX include directory
# GMPXX_LIBRARIES - Libraries needed to use GMPXX
# GMPXX needs GMP
#
# Changes for Cadabra:
#   - Added rudimentary Windows support for building with vcpkg.

if (WIN32)
  windows_find_library(MPIR "mpir" "")
  set(GMPXX_LIBRARIES ${MPIR_LIBRARIES})
  set(GMPXX_BINARIES ${MPIR_BINARIES})
else()
  find_package( GMP QUIET )

  if(GMP_FOUND)
    if (GMPXX_INCLUDE_DIR AND GMPXX_LIBRARIES)
	  # Already in cache, be silent
	  set(GMPXX_FIND_QUIETLY TRUE)
    endif()

    find_path(GMPXX_INCLUDE_DIR 
      NAMES 
	    gmpxx.h 
      PATHS 
	    ${GMP_INCLUDE_DIR_SEARCH}
	  DOC 
	    "The directory containing the GMPXX include files"
    )

    find_library(GMPXX_LIBRARIES 
      NAMES 
	    gmpxx
	  PATHS 
	    ${GMP_LIBRARIES_DIR_SEARCH}
	  DOC 
	    "Path to the GMPXX library"
    )  
	  
    find_package_handle_standard_args(GMPXX "DEFAULT_MSG" GMPXX_LIBRARIES GMPXX_INCLUDE_DIR )
  endif()

endif()
