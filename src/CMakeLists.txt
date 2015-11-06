option(ENABLE_WEBCAM "enable webcam [default ON]" ON)

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(linux/videodev2.h HAVE_VIDEODEV2)
if(HAVE_VIDEODEV2 AND ENABLE_WEBCAM)
  set(ENABLE_WEBCAM ON)
else()
  set(ENABLE_WEBCAM OFF) # disable webcam if v4l2 is not found
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
