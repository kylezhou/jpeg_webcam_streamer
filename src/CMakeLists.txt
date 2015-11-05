include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(linux/videodev2.h HAVE_VIDEODEV2)
if(HAVE_VIDEODEV2)
  set(JPEG_TEST FALSE)
else()
  set(JPEG_TEST TRUE)
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
