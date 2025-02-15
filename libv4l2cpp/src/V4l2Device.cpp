/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2Device.cpp
** 
** -------------------------------------------------------------------------*/

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

// libv4l2
#include <linux/videodev2.h>

#include "logger.h"

#include "V4l2Device.h"

std::string V4l2Device::fourcc(unsigned int format) {
	char formatArray[] = { (char)(format&0xff), (char)((format>>8)&0xff), (char)((format>>16)&0xff), (char)((format>>24)&0xff), 0 };
	return std::string(formatArray, strlen(formatArray));
}

unsigned int V4l2Device::fourcc(const char* format) {
	char fourcc[4];
	memset(&fourcc, 0, sizeof(fourcc));
	if (format != NULL)
		strncpy(fourcc, format, 4);	

	return v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);	
}

// -----------------------------------------
//    V4L2Device
// -----------------------------------------
V4l2Device::V4l2Device(const V4L2DeviceParameters&  params, v4l2_buf_type deviceType) : m_params(params), m_fd(-1), m_deviceType(deviceType), m_bufferSize(0), m_format(0)
{
}

V4l2Device::~V4l2Device() 
{
	this->close();
}

void V4l2Device::close() 
{
	if (m_fd != -1) 		
		::close(m_fd);
		
	m_fd = -1;
}

// query current format
void V4l2Device::queryFormat()
{
	struct v4l2_format     fmt;
	memset(&fmt,0,sizeof(fmt));
	fmt.type  = m_deviceType;
	if (0 == ioctl(m_fd,VIDIOC_G_FMT,&fmt)) 
	{
		m_format     = fmt.fmt.pix.pixelformat;
		m_width      = fmt.fmt.pix.width;
		m_height     = fmt.fmt.pix.height;
		m_bufferSize = fmt.fmt.pix.sizeimage;

		LOG(DEBUG) << m_params.m_devName << ":" << fourcc(m_format) << " size:" << m_width << "x" << m_height << " bufferSize:" << m_bufferSize << std::flush;
	}
}

// intialize the V4L2 connection
bool V4l2Device::init(unsigned int mandatoryCapabilities)
{
	struct stat sb;
	if ((stat(m_params.m_devName.c_str(), &sb)==0) && ((sb.st_mode & S_IFMT) == S_IFCHR))
		if (initdevice(m_params.m_devName.c_str(), mandatoryCapabilities) == -1)
			LOG(ERROR) << "Cannot init device:" << m_params.m_devName << std::flush;
	else
		// open a normal file
		m_fd = open(m_params.m_devName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);

	return (m_fd!=-1);
}

// intialize the V4L2 device
int V4l2Device::initdevice(const char *dev_name, unsigned int mandatoryCapabilities)
{
	m_fd = open(dev_name,  m_params.m_openFlags);
	if (m_fd < 0) 
	{
		LOG(ERROR) << "Cannot open device:" << m_params.m_devName << " " << strerror(errno) << std::flush;
		this->close();
		return -1;
	}

	if (checkCapabilities(m_fd,mandatoryCapabilities) !=0) 
	{
		this->close();
		return -1;
	}	
	
	if (configureFormat(m_fd) !=0) 
	{
		this->close();
		return -1;
	}

	if (setFPS(m_params.m_fps) !=0) 
	{
		this->close();
		return -1;
	}

	if (configureExposureMode(m_fd, m_params.m_exposure_mode) !=0) 
	{
		this->close();
		return -1;
	}
	
	return m_fd;
}

// check needed V4L2 capabilities
int V4l2Device::checkCapabilities(int fd, unsigned int mandatoryCapabilities)
{
	struct v4l2_capability cap;
	memset(&(cap), 0, sizeof(cap));
	if (-1 == ioctl(fd, VIDIOC_QUERYCAP, &cap)) 
	{
		LOG(ERROR) << "Cannot get capabilities for device:" << m_params.m_devName << " " << strerror(errno) << std::flush;
		return -1;
	}

	LOG(DEBUG) << "driver:" << cap.driver << " capabilities:" << std::hex << cap.capabilities <<  " mandatory:" << mandatoryCapabilities << std::dec << std::flush;
		
	if ((cap.capabilities & V4L2_CAP_VIDEO_OUTPUT))  LOG(DEBUG) << m_params.m_devName << " support output" << std::flush;
	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) LOG(DEBUG) << m_params.m_devName << " support capture" << std::flush;

	if ((cap.capabilities & V4L2_CAP_READWRITE))     LOG(DEBUG) << m_params.m_devName << " support read/write" << std::flush;
	if ((cap.capabilities & V4L2_CAP_STREAMING))     LOG(DEBUG) << m_params.m_devName << " support streaming" << std::flush;

	if ((cap.capabilities & V4L2_CAP_TIMEPERFRAME))  LOG(DEBUG) << m_params.m_devName << " support timeperframe" << std::flush;
	
	if ((cap.capabilities & mandatoryCapabilities) != mandatoryCapabilities) 
	{
		LOG(ERROR) << "Mandatory capability not available for device:" << m_params.m_devName << std::flush;
		return -1;
	}
	
	return 0;
}

// configure capture format 
int V4l2Device::configureFormat(int fd)
{
	// get current configuration
	this->queryFormat();		

	unsigned int width = m_width;
	unsigned int height = m_height;
	if (m_params.m_width != 0)
		width= m_params.m_width;
	
	if (m_params.m_height != 0)
		height= m_params.m_height;
	
	if ((m_params.m_formatList.size()==0) && (m_format != 0))  
		m_params.m_formatList.push_back(m_format);
	
	// try to set format, widht, height
	std::list<unsigned int>::iterator it;
	for (it = m_params.m_formatList.begin(); it != m_params.m_formatList.end(); ++it) 
	{
		unsigned int format = *it;
		if (this->configureFormat(fd, format, width, height)==0) 
		{
			// format has been set
			// get the format again because calling SET-FMT return a bad buffersize using v4l2loopback
			this->queryFormat();		
			return 0;
		}
	}

	return -1;
}

// configure capture format 
int V4l2Device::configureFormat(int fd, unsigned int format, unsigned int width, unsigned int height)
{
	struct v4l2_format   fmt;			
	memset(&(fmt), 0, sizeof(fmt));
	fmt.type                = m_deviceType;

	LOG(DEBUG) << "Configuring camera format" << std::flush;

	if (ioctl(m_fd,VIDIOC_G_FMT,&fmt) == -1) 
	{
		LOG(ERROR) << m_params.m_devName << ": Cannot get format " << strerror(errno) << std::flush;
		return -1;
	}

	if (width != 0) 
		fmt.fmt.pix.width       = width;
	if (height != 0) 
		fmt.fmt.pix.height      = height;
	if (format != 0)
		fmt.fmt.pix.pixelformat = format;
	
	if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) 
	{
		LOG(ERROR) << m_params.m_devName << ": Cannot set format:" << fourcc(format) << " " << strerror(errno) << std::flush;
		return -1;
	}

	if (fmt.fmt.pix.pixelformat != format) 
	{
		LOG(ERROR) << m_params.m_devName << ": Cannot set pixelformat to:" << fourcc(format) << " format is:" << fourcc(fmt.fmt.pix.pixelformat) << std::flush;
		return -1;
	}
	
	if ((fmt.fmt.pix.width != width) || (fmt.fmt.pix.height != height))
		LOG(WARN) << m_params.m_devName << ": Cannot set size to:" << width << "x" << height << " size is:"  << fmt.fmt.pix.width << "x" << fmt.fmt.pix.height << std::flush;
	
	m_format     = fmt.fmt.pix.pixelformat;
	m_width      = fmt.fmt.pix.width;
	m_height     = fmt.fmt.pix.height;		
	m_bufferSize = fmt.fmt.pix.sizeimage;
	
	LOG(DEBUG) << m_params.m_devName << ":" << fourcc(m_format) << " size:" << m_width << "x" << m_height << " bufferSize:" << m_bufferSize << std::flush;
	
	return 0;
}

int V4l2Device::configureExposureMode(int fd, V4l2ExposureMode exposure_mode)
{
	struct v4l2_control control;
	control.id = V4L2_CID_EXPOSURE_AUTO;

	switch (exposure_mode) {
	case V4l2ExposureMode::Auto:
		control.value = V4L2_EXPOSURE_APERTURE_PRIORITY; //Fuck zGPT

		LOG(DEBUG) << "Setting exposure_mode: V4L2_EXPOSURE_APERTURE_PRIORITY" << std::flush;

		if(ioctl(fd, VIDIOC_S_CTRL, &control) == -1) 
		{
			if (errno != EINVAL)
				LOG(WARN) << "Quering exposure auto control failed" << std::flush;
			else
				LOG(WARN) << "Exposure auto control not supported" << std::flush;

			return -1;
		}
		break;
	
	case V4l2ExposureMode::Manual:
		control.value = V4L2_EXPOSURE_MANUAL;

		LOG(DEBUG) << "Setting exposure_mode: V4L2_EXPOSURE_MANUAL" << std::flush;

		if(ioctl(fd, VIDIOC_S_CTRL, &control) == -1) 
		{
			LOG(WARN) << "Exposure control to manual mode failed" << std::flush; 
			return -1;
		}
		break;
	}
	return 0;
};

int V4l2Device::setExposureTime(int time_ms)
{
	struct v4l2_control ctrl;
	ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
	ctrl.value = time_ms * 10; // 30ms = 300

	LOG(DEBUG) << "Setting exposure_time_absolute: " << ctrl.value << " (" << time_ms << "ms)" << std::flush;

	if (ioctl(getFd(), VIDIOC_S_CTRL, &ctrl) == -1) 
	{
		LOG(WARN) << "Setting exposure time failed" << std::flush; 
		return -1;
	}

	return 0;
}


int V4l2Device::setFPS(int fps)
{
	LOG(DEBUG) << "Setting FPS: " << fps << std::flush;

	if (fps > 0) {
		struct v4l2_streamparm  param;			
		memset(&(param), 0, sizeof(param));
		param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		param.parm.capture.timeperframe.numerator = 1;
		param.parm.capture.timeperframe.denominator = fps;
		
		if (ioctl(getFd(), VIDIOC_S_PARM, &param) == -1) 
		{
			LOG(WARN) << "Cannot update device FPS:" << std::flush;
			LOG(WARN) << "Cannot set param for device:" << m_params.m_devName << " " << strerror(errno) << std::flush;
			return -1;
		}

		LOG(DEBUG) << "Updated device FPS timeperframe.numerator " << param.parm.capture.timeperframe.numerator << " timeperframe.denominator " << param.parm.capture.timeperframe.denominator << std::flush;
	}
	return 0;
}


