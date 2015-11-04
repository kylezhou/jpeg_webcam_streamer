bin_PROGRAMS = WebcamStreamer
WebcamStreamer_SOURCES = JpegFrameParser.cpp JpegFrameParser.hh WebcamJPEGDeviceSource.cpp WebcamJPEGDeviceSource.hh WebcamStreamer.cpp
WebcamStreamer_CXXFLAGS = $(LIVE555_CFLAGS) -DNDEBUG
WebcamStreamer_LDADD = $(LIVE555_LIBS)
if !HAVE_VIDEODEV2
    WebcamStreamer_CXXFLAGS += -DJPEG_TEST
endif
