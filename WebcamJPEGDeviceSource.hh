/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**********/
// Webcam MJPEG camera input device
// C++ header

#ifndef _WEBCAM_JPEG_DEVICE_SOURCE_HH
#define _WEBCAML_JPEG_DEVICE_SOURCE_HH

#ifndef _JPEG_VIDEO_SOURCE_HH
#include "JPEGVideoSource.hh"
#endif

#include "JpegFrameParser.hh"

#define JPEG_HEADER_SIZE 623

#include <exception>
class DeviceException : public std::exception {
    
};

class WebcamJPEGDeviceSource: public JPEGVideoSource {
public:
  static WebcamJPEGDeviceSource* createNew(UsageEnvironment& env,
					   unsigned timePerFrame);
  // "timePerFrame" is in microseconds

protected:
  WebcamJPEGDeviceSource(UsageEnvironment& env,
			 int fd, unsigned timePerFrame);
  // called only by createNew()
  virtual ~WebcamJPEGDeviceSource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual u_int8_t type();
  virtual u_int8_t qFactor();
  virtual u_int8_t width();
  virtual u_int8_t height();
  virtual u_int8_t const * quantizationTables(u_int8_t & precision, u_int16_t & length);

private:
  int initDevice(UsageEnvironment& env, int fd);

    struct buffer {
        void   *start;
        size_t  length;
    };

    size_t jpeg_to_rtp(void *to, void *from, size_t len);
    
private:
  int fFd;
  unsigned fTimePerFrame;
  struct timeval fLastCaptureTime;
  u_int8_t fLastQFactor, fLastWidth, fLastHeight;
  Boolean fNeedAFrame;
  unsigned char fJPEGHeader[JPEG_HEADER_SIZE];
    struct buffer *fBuffers;
    unsigned int fNbuffers;
    unsigned char *payload;
    size_t payload_sz;
    
    JpegFrameParser parser;

};

#endif
