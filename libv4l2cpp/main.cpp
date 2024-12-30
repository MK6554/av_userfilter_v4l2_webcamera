/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** main.cpp
** 
** test V4L2 capture device 
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "logger.h"
#include "V4l2Capture.h"

int stop=0;

/* ---------------------------------------------------------------------------
**  SIGINT handler
** -------------------------------------------------------------------------*/
void sighandler(int)
{ 
       printf("SIGINT\n");
       stop =1;
}

/* ---------------------------------------------------------------------------
**  main
** -------------------------------------------------------------------------*/
int main(int argc, char* argv[]) 
{	
	int verbose = 0;
	const char *in_devname = "/dev/video0";	
	V4l2IoType ioTypeIn  = V4l2IoType::IOTYPE_MMAP;
	int format = v4l2_fourcc('M', 'J', 'P', 'G');
	int width = 2592;
	int height = 1944;
	int fps = 30;
	int framecount = 0;
	int c = 0;

	// initialize log4cpp
	initLogger(verbose);


	while ((c = getopt (argc, argv, "x:hv:" "G:f:r")) != -1)
	{
		switch (c)
		{
			case 'v':	verbose   = 1; if (optarg && *optarg=='v') verbose++; break;
			case 'r':	ioTypeIn  = IOTYPE_READWRITE                        ; break;			
            case 'G':   sscanf(optarg,"%dx%dx%d", &width, &height, &fps)    ; break;
			case 'f':	format    = V4l2Device::fourcc(optarg)              ; break;
			case 'x':   sscanf(optarg,"%d", &framecount) 					; break;
			case 'h':
			{
				std::cout << argv[0] << " [-v[v]] [-G <width>x<height>x<fps>] [-f format] [device] [-r]" << std::endl;
				std::cout << "\t -G <width>x<height>x<fps> : set capture resolution" << std::endl;
				std::cout << "\t -v            : verbose " << std::endl;
				std::cout << "\t -vv           : very verbose " << std::endl;
				std::cout << "\t -r            : V4L2 capture using read interface (default use memory mapped buffers)" << std::endl;
				std::cout << "\t -x <count>    : read <count> frames and save them in current dir." << std::endl;
				std::cout << "\t device        : V4L2 capture device (default "<< in_devname << ")" << std::endl;
				exit(0);
			}
		}
	}
	if (optind<argc)
	{
		in_devname = argv[optind];
		optind++;
	}	


	// init V4L2 capture interface
	V4L2DeviceParameters param(in_devname, format, width, height, fps, ioTypeIn);
	V4l2Capture* videoCapture = V4l2Capture::create(param);
	
	if (videoCapture == NULL)
	{	
		LOG(WARN) << "Cannot reading from V4L2 capture interface for device:" << in_devname; 
	}
	else
	{
		timeval tv;

		LOG(NOTICE) << "Start reading from " << in_devname;
		LOG(NOTICE) << "Buffer Size: " << videoCapture->getBufferSize() << " Bytes"; 

		char* buffer = new char[videoCapture->getBufferSize()];
		
		//signal(SIGINT,sighandler);				
		while (!stop) 
		{
			tv.tv_sec=1;
			tv.tv_usec=0;

			if (videoCapture->isReadable(&tv))
			{
				int rsize = videoCapture->read(buffer, videoCapture->getBufferSize());

				if (rsize == -1)
				{
					LOG(NOTICE) << "stop " << strerror(errno); 
					stop=1;					
				}
				else
				{
					LOG(NOTICE) << "size:\t" << rsize;
					static int stop_count = 0;
					if(framecount && stop_count < framecount){
						std::string filename = "Frame" + std::to_string(stop_count) + '.' + V4l2Device::fourcc(videoCapture->getFormat());
						stop_count++;


						FILE *fp = fopen(filename.c_str(), "wb");
						fwrite(buffer, 1, rsize, fp);
						fclose(fp);
						LOG(NOTICE) << "saved:\t" << filename;
					}
				}
			}
			else
			{
				LOG(NOTICE) << "stop " << strerror(errno); 
				stop=1;
			}
		}
		
		delete buffer;
		delete videoCapture;
	}
	
	return 0;
}