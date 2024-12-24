/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Device.h
** 
** V4L2 wrapper 
**
** -------------------------------------------------------------------------*/


#pragma once

#include <string>
#include <list>
#include <linux/videodev2.h>
#include <fcntl.h>

#include <mutex>

#ifndef V4L2_PIX_FMT_VP8
#define V4L2_PIX_FMT_VP8  v4l2_fourcc('V', 'P', '8', '0')
#endif

#ifndef V4L2_PIX_FMT_VP9
#define V4L2_PIX_FMT_VP9  v4l2_fourcc('V', 'P', '9', '0')
#endif

#ifndef V4L2_PIX_FMT_HEVC
#define V4L2_PIX_FMT_HEVC  v4l2_fourcc('H', 'E', 'V', 'C')
#endif


#ifndef V4L2_PIX_FMT_HEVC
#define V4L2_PIX_FMT_MJPG v4l2_fourcc('M', 'J', 'P', 'G')
#endif


enum V4l2IoType
{
	IOTYPE_READWRITE,
	IOTYPE_MMAP
};


enum V4l2ExposureMode
{
	Manual,
	Auto
};

// ---------------------------------
// V4L2 Device parameters
// ---------------------------------
struct V4L2DeviceParameters 
{
	V4L2DeviceParameters(const char* devname, const std::list<unsigned int> & formatList, unsigned int width, unsigned int height, int fps, int openFlags = O_RDWR | O_NONBLOCK, V4l2IoType ioType = IOTYPE_MMAP, V4l2ExposureMode exposure_mode = V4l2ExposureMode::Auto) : 
		m_devName(devname), m_formatList(formatList), m_width(width), m_height(height), m_fps(fps), m_iotype(ioType), m_openFlags(openFlags), m_exposure_mode(exposure_mode) {}

	V4L2DeviceParameters(const char* devname, unsigned int format, unsigned int width, unsigned int height, int fps, V4l2IoType ioType = IOTYPE_MMAP, int openFlags = O_RDWR | O_NONBLOCK, V4l2ExposureMode exposure_mode = V4l2ExposureMode::Auto) : 
		m_devName(devname), m_width(width), m_height(height), m_fps(fps), m_iotype(ioType), m_openFlags(openFlags), m_exposure_mode(exposure_mode) {
			if (format) {
				m_formatList.push_back(format);
			}
	}
		
	std::string m_devName;

	V4l2ExposureMode m_exposure_mode = V4l2ExposureMode::Auto;

	std::list<unsigned int> m_formatList;
	unsigned int m_width;
	unsigned int m_height;
	int m_fps;			
	V4l2IoType m_iotype;
	int m_verbose;
	int m_openFlags;
};

// ---------------------------------
// V4L2 Device
// ---------------------------------
class V4l2Device
{		
	friend class V4l2Capture;
	friend class V4l2Output;
	
	protected:	
		void close();	
		
		int initdevice(const char *dev_name, unsigned int mandatoryCapabilities);
		int checkCapabilities(int fd, unsigned int mandatoryCapabilities);
		int configureFormat(int fd);
		int configureFormat(int fd, unsigned int format, unsigned int width, unsigned int height);
		int setFPS(int fps);

		int configureExposureMode(int fd, V4l2ExposureMode exposure_mode);

		
		virtual size_t writeInternal(char*, size_t)        { return -1;    }
		virtual bool   startPartialWrite()                 { return false; }
		virtual size_t writePartialInternal(char*, size_t) { return -1;    }
		virtual bool   endPartialWrite()                   { return false; }
		virtual size_t readInternal(char*, size_t)         { return -1;    }
	
	public:
		virtual bool   init(unsigned int mandatoryCapabilities);		

		V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType);		
		virtual ~V4l2Device();
	
		virtual bool isReady() { return (m_fd != -1); }
		virtual bool start()   { return true; }
		virtual bool stop()    { return true; }
	
		unsigned int getBufferSize() { return m_bufferSize; }
		unsigned int getFormat()     { return m_format;     }
		unsigned int getWidth()      { return m_width;      }
		unsigned int getHeight()     { return m_height;     }
		int          getFd()         { return m_fd;         }
		void         queryFormat();
			
		int setFormat(unsigned int format, unsigned int width, unsigned int height) {
			return this->configureFormat(m_fd, format, width, height);
		}

		int setExposureTime(int time);

		int setFps(int fps) {
			return this->setFPS(fps);
		}	

		int setExposureMode(V4l2ExposureMode exposure_mode)
		{
			return this->configureExposureMode(m_fd, exposure_mode);
		}

		static std::string fourcc(unsigned int format);
		static unsigned int fourcc(const char* format);
		
	protected:
		V4L2DeviceParameters m_params;
		int m_fd;
		v4l2_buf_type m_deviceType;	

		V4l2ExposureMode m_exposure = V4l2ExposureMode::Auto;
	
		unsigned int m_bufferSize;
		unsigned int m_format;
		unsigned int m_width;
		unsigned int m_height;	

		struct v4l2_buffer m_partialWriteBuf;
		bool m_partialWriteInProgress;
};


