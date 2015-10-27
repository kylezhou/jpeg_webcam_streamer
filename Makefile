# set up basic variables
CC = g++
CFLAGS = -c -Wall
LDFLAGS =

# list of sources
SOURCES = JpegFrameParser.cpp WebcamJPEGDeviceSource.cpp WebcamStreamer.cpp
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = JpegFrameParser.hh WebcamJPEGDeviceSource.hh

# name of executable target
EXECUTABLE = WebcamStreamer

# live555 specific flags
CFLAGS += `pkg-config --cflags live555`
LDFLAGS += `pkg-config --libs live555`

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o : %.cpp $(DEPS)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
