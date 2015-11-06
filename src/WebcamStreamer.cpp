/*
 Copyright (C) 2015, Kyle Zhou <kyle.zhou at live.com>
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// A test program that reads JPEG frames from a webcam
// and streams them via RTP/RTCP
// main program

#include "liveMedia.hh"
#include "GroupsockHelper.hh"
#include "BasicUsageEnvironment.hh"

#include "WebcamJPEGDeviceSource.hh"

#include <unistd.h> /* GNU getopt */

UsageEnvironment* env;
char* progName;
int fps;
char const * testJpeg = NULL;
char const * camDev = "/dev/video0";

void play(); // forward

void usage()
{
    *env << "Usage: " << progName << " -f <frames-per-second> [-d <webcam-dev-file>] [-t <test-jpeg-file>]\n";
    exit(1);
}

int main(int argc, char** argv)
{
    // Begin by setting up our usage environment:
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);

    //OutPacketBuffer::numPacketsLimit = 100;
    // Allow for up to 100 RTP packets per JPEG frame
    
    int c;
    progName = argv[0];
    opterr = 0;
    char tmp[2] = {0,0};
    while((c=getopt(argc, argv, "d:f:t:h")) != -1)
    {
        switch(c) {
        case 'd':
#ifndef ENABLE_WEBCAM
            *env << "Webcam support is not included in this program.\n";
            return 3;
#else
            camDev = optarg;
#endif
            break;
            
        case 'f':
            if(sscanf(optarg, "%d", &fps)!=1 || fps <= 0) {
                tmp[0]=optopt;
                *env << "Option -" << tmp << " has illegal value " << optarg << ".\n";
                usage();
            }
            break;
            
        case 't':
            testJpeg = optarg;
            break;
        
        case 'h':
            usage();
            break;
            
        case '?':
            tmp[0] = optopt;
            if(optopt == 'f' or optopt == 't' or optopt == 'd')
                *env << "Option -" << tmp << " requires an argument.\n";
            else if(isprint(optopt))
                *env << "Unknown option -"<< tmp <<".\n";
            else
                *env << "Unknown option character \\" << optopt << ".\n";
            
        default:
            usage();
        }
    }
    for(int i=optind; i<argc; i++)
        *env<<"Non-option argument "<<argv[i]<<".\n";
    if(optind < argc) {
        *env<<"There are non-option arguments\n";
        usage();
    }
#ifndef ENABLE_WEBCAM
    if(!testJpeg) {
        *env << "Your have to specify a test jpeg file since webcam support is not included in this program.\n";
        usage();
    }
#endif
    if(fps<=0)
    {
        *env<<"You have to specify a valid fps.\n";
        usage();
    }
    *env << "camDev="<<camDev<<", fps=" << fps << ", testJpeg=" << testJpeg << ".\n";

    play();

    return 0;
}

void afterPlaying(void* clientData); // forward

// A structure to hold the state of the current session.
// It is used in the "afterPlaying()" function to clean up the session.
struct sessionState_t {
    FramedSource* source;
    RTPSink* sink;
    RTCPInstance* rtcpInstance;
    Groupsock* rtpGroupsock;
    Groupsock* rtcpGroupsock;
    RTSPServer* rtspServer;
} sessionState;

void play() {
    // Open the webcam
    unsigned timePerFrame = 1000000/fps; // microseconds
    sessionState.source
        = WebcamJPEGDeviceSource::createNew(*env, timePerFrame, camDev, testJpeg);
    if (sessionState.source == NULL) {
        *env << env->getResultMsg() << "\n";
        exit(1);
    }

    // Create 'groupsocks' for RTP and RTCP:
    struct in_addr destinationAddress;
    destinationAddress.s_addr = chooseRandomIPv4SSMAddress(*env);

    const unsigned short rtpPortNum = 16384;
    const unsigned short rtcpPortNum = rtpPortNum+1;
    const unsigned char ttl = 255;
  
    const Port rtpPort(rtpPortNum);
    const Port rtcpPort(rtcpPortNum);
  
    sessionState.rtpGroupsock
        = new Groupsock(*env, destinationAddress, rtpPort, ttl);
    sessionState.rtpGroupsock->multicastSendOnly(); // we're a SSM source
    sessionState.rtcpGroupsock
        = new Groupsock(*env, destinationAddress, rtcpPort, ttl);
    sessionState.rtcpGroupsock->multicastSendOnly(); // we're a SSM source
  
    // Create an appropriate RTP sink from the RTP 'groupsock':
    sessionState.sink
        = JPEGVideoRTPSink::createNew(*env, sessionState.rtpGroupsock);
  
    // Create (and start) a 'RTCP instance' for this RTP sink:
    unsigned const averageFrameSizeInBytes = 35000; // estimate
    const unsigned totalSessionBandwidth
        = (8*1000*averageFrameSizeInBytes)/timePerFrame;
        // in kbps; for RTCP b/w share
    const unsigned maxCNAMElen = 100;
    unsigned char CNAME[maxCNAMElen+1];
    //gethostname((char*)CNAME, maxCNAMElen);
    sprintf((char*)CNAME, "Webcam"); // "gethostname()" isn't supported
    CNAME[maxCNAMElen] = '\0'; // just in case
    sessionState.rtcpInstance
        = RTCPInstance::createNew(*env, sessionState.rtcpGroupsock,
			      totalSessionBandwidth, CNAME,
			      sessionState.sink, NULL /* we're a server */,
			      True /* we're a SSM source*/);
    // Note: This starts RTCP running automatically

    // Create and start a RTSP server to serve this stream:
    sessionState.rtspServer
        = RTSPServer::createNew(*env, 7070);
    if (sessionState.rtspServer == NULL) {
        *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }
    ServerMediaSession* sms
        = ServerMediaSession::createNew(*env, NULL, progName,
            "Session streamed by the Webcam", True/*SSM*/);
    sms->addSubsession(PassiveServerMediaSubsession
		    ::createNew(*sessionState.sink));
    sessionState.rtspServer->addServerMediaSession(sms);
 
    char* url = sessionState.rtspServer->rtspURL(sms);
    *env << "Play this stream using the URL \"" << url << "\"\n";
    delete[] url;

    // Finally, start the streaming:
    *env << "Beginning streaming...\n";
    sessionState.sink->startPlaying(*sessionState.source, afterPlaying, NULL);

    env->taskScheduler().doEventLoop();
}


void afterPlaying(void* /*clientData*/)
{
    *env << "...done streaming\n";

    // End by closing the media:
    Medium::close(sessionState.rtspServer);
    Medium::close(sessionState.sink);
    delete sessionState.rtpGroupsock;
    Medium::close(sessionState.source);
    Medium::close(sessionState.rtcpInstance);
    delete sessionState.rtcpGroupsock;

    // We're done:
    exit(0);
}
