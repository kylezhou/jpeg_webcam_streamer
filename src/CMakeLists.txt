option(ENABLE_WEBCAM "enable webcam [default ON]" ON)

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES("linux/videodev2.h;fcntl.h;sys/mman.h;sys/ioctl.h" HAVE_VIDEODEV2)
if(NOT HAVE_VIDEODEV2)
  set(ENABLE_WEBCAM OFF) # disable webcam if v4l2 is not found
endif()

CHECK_INCLUDE_FILES(sys/time.h HAVE_SYS_TIME)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD)

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX(algorithm HAVE_ALGORITHM)
CHECK_INCLUDE_FILE_CXX(cstring HAVE_CSTRING)
CHECK_INCLUDE_FILE_CXX(cstdio HAVE_CSTDIO)
CHECK_INCLUDE_FILE_CXX(exception HAVE_EXCEPTION)

if( NOT(HAVE_SYS_TIME AND HAVE_UNISTD AND HAVE_ALGORITHM AND 
       HAVE_CSTRING AND HAVE_CSTDIO AND HAVE_EXCEPTION) )
  message(FATAL_ERROR "essential header files are missing.")
endif()

option(ENABLE_DEBUG "enable debug mode [default OFF]" OFF)
if(NOT ENABLE_DEBUG)
  set(NDEBUG ON)
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Live555 REQUIRED)
include_directories(${LIVE555_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LIVE555_LIBRARIES})

add_executable(WebcamStreamer WebcamStreamer.cpp WebcamJPEGDeviceSource.cpp WebcamJPEGDeviceSource.hh JpegFrameParser.cpp JpegFrameParser.hh)

target_link_libraries(WebcamStreamer ${LIBS})
