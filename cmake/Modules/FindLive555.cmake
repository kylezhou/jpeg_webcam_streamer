# - Try to find Live555
# Once done this will define
# LIVE555_FOUND - System has Live555
# LIVE555_INCLUDE_DIRS - The Live555 include directories
# LIVE555_LIBRARIES - The libraries needed to use Live555
# LIVE555_DEFINITIONS - Compiler switches required for using Live555

include(FindPkgConfig)
find_package(PkgConfig)
pkg_check_modules(PC_LIVE555 QUIET live555)
set(LIVE555_DEFINITIONS ${PC_LIVE555_CFLAGS_OTHER})

find_path(LIVE555_MEDIA_INCLUDE_DIR liveMedia.hh
          HINTS ${PC_LIVE555_INCLUDEDIR} ${PC_LIVE555_INCLUDE_DIRS}
          PATHS /opt/local/lib/
          PATH_SUFFIXES live/liveMedia/include )
find_library(LIVE555_MEDIA_LIBRARY NAMES liveMedia
             HINTS ${PC_LIVE555_LIBDIR} ${PC_LIVE555_LIBRARY_DIRS}
             PATH_SUFFIXES live/liveMedia )

find_path(LIVE555_GROUP_INCLUDE_DIR Groupsock.hh
          HINTS ${PC_LIVE555_INCLUDEDIR} ${PC_LIVE555_INCLUDE_DIRS}
          PATHS /opt/local/lib/
          PATH_SUFFIXES live/groupsock/include )
find_library(LIVE555_GROUP_LIBRARY NAMES groupsock
             HINTS ${PC_LIVE555_LIBDIR} ${PC_LIVE555_LIBRARY_DIRS}
             PATH_SUFFIXES live/groupsock )

find_path(LIVE555_UENV_INCLUDE_DIR UsageEnvironment.hh
          HINTS ${PC_LIVE555_INCLUDEDIR} ${PC_LIVE555_INCLUDE_DIRS}
          PATHS /opt/local/lib/
          PATH_SUFFIXES live/UsageEnvironment/include )
find_library(LIVE555_UENV_LIBRARY NAMES UsageEnvironment
             HINTS ${PC_LIVE555_LIBDIR} ${PC_LIVE555_LIBRARY_DIRS}
             PATH_SUFFIXES live/UsageEnvironment )

find_path(LIVE555_BUENV_INCLUDE_DIR BasicUsageEnvironment.hh
          HINTS ${PC_LIVE555_INCLUDEDIR} ${PC_LIVE555_INCLUDE_DIRS}
          PATHS /opt/local/lib/
          PATH_SUFFIXES live/BasicUsageEnvironment/include )
find_library(LIVE555_BUENV_LIBRARY NAMES BasicUsageEnvironment
             HINTS ${PC_LIVE555_LIBDIR} ${PC_LIVE555_LIBRARY_DIRS}
             PATH_SUFFIXES live/BasicUsageEnvironment )

set(LIVE555_LIBRARIES ${LIVE555_MEDIA_LIBRARY} ${LIVE555_GROUP_LIBRARY} ${LIVE555_UENV_LIBRARY} ${LIVE555_BUENV_LIBRARY})
set(LIVE555_INCLUDE_DIRS ${LIVE555_MEDIA_INCLUDE_DIR} ${LIVE555_GROUP_INCLUDE_DIR} ${LIVE555_UENV_INCLUDE_DIR} ${LIVE555_BUENV_INCLUDE_DIR})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( Live555 DEFAULT_MSG 
  LIVE555_MEDIA_LIBRARY LIVE555_MEDIA_INCLUDE_DIR
  LIVE555_GROUP_LIBRARY LIVE555_GROUP_INCLUDE_DIR
  LIVE555_UENV_LIBRARY LIVE555_UENV_INCLUDE_DIR
  LIVE555_BUENV_LIBRARY LIVE555_BUENV_INCLUDE_DIR )
mark_as_advanced(LIVE555_MEDIA_LIBRARY LIVE555_MEDIA_INCLUDE_DIR
  LIVE555_GROUP_LIBRARY LIVE555_GROUP_INCLUDE_DIR
  LIVE555_UENV_LIBRARY LIVE555_UENV_INCLUDE_DIR
  LIVE555_BUENV_LIBRARY LIVE555_BUENV_INCLUDE_DIR )
