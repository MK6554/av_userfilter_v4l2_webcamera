#include "main.hpp"
#include "debug_log.hpp"

#include "webcamera_manager.hpp"

#include <iostream>
#include <thread>

#define FRAMERATE_MAX 333.0 //Framerate high enough to not interfere with natural FPS (should wait for 3ms)

namespace avs
{
#define THROW_NOT_CONNECTED throw atl::IoError("Attempted to reuse a device that does not exist.")

	void WebCameraBase::disconnect()
	{
		auto dev = WebCameraManager::instance()->get_device(m_camera_index);
		if (dev)
		{
			dev->close_acquisition();
			WebCameraManager::instance()->remove_device(m_camera_index);
		}
	}

	void WebCameraBase::add_camera_params()
	{
		AddInput(L"inCameraIndex", L"Integer<0, INF>*", L"", L"Device index.");
	}

	void WebCameraBase::read_inputs()
	{
		atl::Optional<int> id;
		ReadInput(L"inCameraIndex", id);
		m_camera_index = id.GetValueOr(0);
	}

	void WebCameraBase_Input::get_or_make_device()
	{
		auto mng = WebCameraManager::instance();
		auto existing = mng->get_device(m_camera_index);
		if (existing)
		{
			m_camera = existing;
			return;
		}
		m_camera = std::make_shared<WebCamera>(m_camera_index, inWidth, inHeight,
											   inFps, inExposure);
		mng->add_device(m_camera);
	}

	void WebCameraBase_Input::add_camera_params()
	{
		WebCameraBase::add_camera_params();
		AddInput(L"inWidth", L"Integer<1, INF>*", L"", L"Image width. Leave auto to use maximum available height.");
		AddInput(L"inHeight", L"Integer<1, INF>*", L"", L"Image height. Leave auto to use maximum available height.");
		AddInput(L"inFPS", L"Integer<0, INF>*", L"", L"FPS limit. Camera will be throttled down to this framerate.");
		AddInput(L"inExposure", L"Integer<10, INF>", L"30", L"Desired exposure time in milliseconds.");
	}

	void WebCameraBase_Input::read_inputs()
	{
		WebCameraBase::read_inputs();
		atl::Optional<int> opt_width, opt_height, opt_framerate;
		ReadInput(L"inWidth", opt_width);
		ReadInput(L"inHeight", opt_height);
		inHeight = opt_height.GetValueOr(-1);
		inWidth = opt_width.GetValueOr(-1);
		ReadInput(L"inFPS", opt_framerate);
		inFps = opt_framerate.GetValueOr(FRAMERATE_MAX);
		ReadInput(L"inExposure", inExposure);
	}

	int WebCameraBase_Input::Init()
	{
		read_inputs();
		get_or_make_device();
		if (!m_camera->is_running())
		{

			m_camera->start_acquisition();
		}
		return INIT_OK;
	}

	void WebCameraStartAcquisition::Define()
	{
		SetName(L"UfWebCamera_StartAcquisition");
		SetTip(L"Starts acquisition of V4L webcamera.");
		add_camera_params();
	}

	int WebCameraStartAcquisition::Invoke() { return INVOKE_LOOP; }

	void WebCameraCloseAcquisition::Define()
	{
		SetName(L"UfWebCamera_CloseAcquisition");
		SetTip(L"Closes acquisition of V4L webcamera.");
		add_camera_params();
	}

	int WebCameraCloseAcquisition::Invoke()
	{
		disconnect();
		return INVOKE_NORMAL;
	}

	void WebCameraGrabImage_Synchronous::Define()
	{
		SetName(L"UfWebCamera_GrabImage_Synchronous");
		SetTip(L"Grabs an image from the V4L camera, optionally starting the acquisition beforehand.");
		add_camera_params();
		AddOutput(L"outImage", L"Image", L"Grabbed image.");
		AddOutput(L"outFPS", L"Real", L"Last reported FPS.");
		AddAttribute(L"FilterGroup", L"UfWebCamera_GrabImage<Synchronous> default ## Acquire images from a V4L webcamera.");
	}

	int WebCameraGrabImage_Synchronous::Invoke()
	{
		while (!m_camera->can_grab())
		{

			if (IsWorkCancelled())
			{
				outImage.Reset();
				WriteOutput(L"outImage", outImage);
				WriteOutput<float>(L"outFPS", 0.0);
				return INVOKE_END;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		m_camera->grab_image(outImage);

		auto fps = (float)m_camera->get_fps();
		WriteOutput(L"outImage", outImage);
		WriteOutput<float>(L"outFPS", fps);
		return INVOKE_LOOP;
	}
	void WebCameraGrabImage_WithTimeout::Define()
	{
		SetName(L"UfWebCamera_GrabImage_WithTimeout");
		SetTip(L"Grabs an image from the V4L camera, optionally starting the acquisition beforehand.");
		AddInput(L"inTimeout", L"Integer<0, INF, 50>", L"100", L"Maximum time to wait in milliseconds.");
		add_camera_params();
		AddOutput(L"outImage", L"Image?", L"Grabbed image.");
		AddOutput(L"outFPS", L"Real?", L"Last reported FPS.");
		AddAttribute(L"FilterGroup", L"UfWebCamera_GrabImage<WithTimeout>");
	}

	int WebCameraGrabImage_WithTimeout::Invoke()
	{
		int timeout_ms;
		ReadInput(L"inTimeout", timeout_ms);
		auto threshold = std::chrono::milliseconds(timeout_ms);
		auto started = std::chrono::system_clock::now();
		while (!m_camera->can_grab())
		{
			auto elapsed = std::chrono::system_clock::now() - started;
			if (IsWorkCancelled() || elapsed > threshold)
			{
				transImage.Reset();
				WriteOutput(L"outImage", atl::NIL);
				WriteOutput(L"outFPS", atl::NIL);
				return INVOKE_LOOP;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		m_camera->grab_image(transImage);
		outImage = transImage;
		auto fps = (float)m_camera->get_fps();
		WriteOutput(L"outImage", outImage);
		WriteOutput(L"outFPS", atl::Conditional<float>(fps));
		return INVOKE_LOOP;
	}
	/*
	SET PARAMETER
	*/
	void WebCameraSetParameter::Define()
	{
		SetName(L"UfWebCamera_SetParameter");
		SetTip(L"Set a generic parameter from the camera.");
		add_camera_params();
		AddInput(L"inParameterId", L"Integer<0, INF>", L"", L"Parameter ID. Parameter IDS match those for OpenCV's VideoCapture (cv::PROP_CAP_*).");
		AddInput(L"inParameterValue", L"Double", L"", L"Parameter value.");
		AddOutput(L"outSuccess", L"Bool", L"Status");
		AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<Generic>");
	}
	int WebCameraSetParameter::Invoke()
	{
		read_inputs();
		int id;
		double val;
		ReadInput(L"inParameterId", id);
		ReadInput(L"inParameterValue", val);
		
		
		switch (id)
		{
		case 15:
			throw atl::DomainError("To set exposure use the dedicated filter.");
		case 5:
			throw atl::DomainError("To limit frame rate use the dedicated filter.");
		}
		
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		//auto res = cam->set_property(id, val);
		//WriteOutput(L"outSuccess", res);
		return INVOKE_NORMAL;
	}
	/*
	SET FRAME RATE LIMIT
	*/
	void WebCameraSetFrameRateLimit::Define()
	{
		SetName(L"UfWebCamera_SetFrameRateLimit");
		SetTip(L"Set new frame rate limit.");
		add_camera_params();
		AddInput(L"inFpsLimit", L"Integer<1,INF>*", L"", L"Fps limit.");
		AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<FrameRate>");
	}
	int WebCameraSetFrameRateLimit::Invoke()
	{
		read_inputs();
		atl::Optional<int> val;
		ReadInput(L"inFpsLimit", val);
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		cam->set_max_framerate(val.GetValueOr(FRAMERATE_MAX));
		return INVOKE_NORMAL;
	}
	/*
	SET EXPOSURE
	*/
	void WebCameraSetExposure::Define()
	{
		SetName(L"UfWebCamera_SetExposure");
		SetTip(L"Set new exposure to camera (if not on auto exposure).");
		add_camera_params();
		AddInput(L"inExposure", L"Integer<10, INF>*", L"30", L"Desired exposure time in milliseconds. Auto enables auto-exposure.");
		AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<Exposure> default ## Modify web camera's parameters.");
	}
	int WebCameraSetExposure::Invoke()
	{
		read_inputs();
		atl::Optional<int> val;
		ReadInput(L"inExposure", val);
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		cam->set_exposure(val.GetValueOr(-1));
		return INVOKE_NORMAL;
	}
	/*
	GET PARAMETER
	*/
	void WebCameraGetParameter::Define()
	{
		SetName(L"UfWebCamera_GetParameter");
		SetTip(L"Get a generic parameter from the camera.");
		add_camera_params();
		AddInput(L"inParameterId", L"Integer<0, INF>", L"0", L"Parameter ID. Parameter IDS match those for OpenCV's VideoCapture (cv::PROP_CAP_*).");
		AddOutput(L"outValue", L"Double", L"Parameter value");
		AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<Generic>");
	}
	int WebCameraGetParameter::Invoke()
	{
		read_inputs();
		int id;
		ReadInput(L"inParameterId", id);
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		switch (id)
		{
		case 15:
			WriteOutput(L"outValue", (double)cam->exposure().count());
			break;
		case 5:
			WriteOutput(L"outValue", (double)cam->max_framerate());
			break;
		default:
			//auto res = cam->get_property(id);
			//WriteOutput(L"outValue", res);
			break;
		}
		return INVOKE_NORMAL;
	}
	/*
	GET FRAME RATE
	*/
	void WebCameraGetFrameRateLimit::Define()
	{
		SetName(L"UfWebCamera_GetFrameRateLimit");
		SetTip(L"Gets current value of frame rate limit.");
		add_camera_params();
		AddOutput(L"outFpsLimit", L"Integer", L"Fps limit.");
		AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<FrameRate>");
	}
	int WebCameraGetFrameRateLimit::Invoke()
	{
		read_inputs();
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		WriteOutput(L"outFpsLimit", cam->max_framerate());
		return INVOKE_NORMAL;
	}
	/*
	GET EXPOSURE
	*/
	void WebCameraGetExposure::Define()
	{
		SetName(L"UfWebCamera_GetExposure");
		SetTip(L"Gets current value of exposure (manually set only).");
		add_camera_params();
		AddOutput(L"outExposure", L"Integer", L"Exposure time in milliseconds");
		AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<Exposure> default ## Read web camera's parameters.");
	}
	int WebCameraGetExposure::Invoke()
	{
		read_inputs();
		auto cam = WebCameraManager::instance()->get_device(m_camera_index);
		if (!cam)
		{
			THROW_NOT_CONNECTED;
		}
		WriteOutput(L"outExposure", (int)cam->exposure().count());
		return INVOKE_NORMAL;
	}

	RegisterUserObjects::RegisterUserObjects()
	{
		RegisterFilter(CreateInstance<WebCameraStartAcquisition>);
		RegisterFilter(CreateInstance<WebCameraGrabImage_Synchronous>);
		RegisterFilter(CreateInstance<WebCameraGrabImage_WithTimeout>);
		RegisterFilter(CreateInstance<WebCameraCloseAcquisition>);

		RegisterFilter(CreateInstance<WebCameraGetParameter>);
		RegisterFilter(CreateInstance<WebCameraGetFrameRateLimit>);
		RegisterFilter(CreateInstance<WebCameraGetExposure>);

		RegisterFilter(CreateInstance<WebCameraSetParameter>);
		RegisterFilter(CreateInstance<WebCameraSetFrameRateLimit>);
		RegisterFilter(CreateInstance<WebCameraSetExposure>);
	}

	static RegisterUserObjects registerUserObjects;

} // namespace avs
