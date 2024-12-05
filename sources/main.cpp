#include <iostream>

#include <AVL_Lite.h>
#include "webcamera.hpp"
#include "webcamera_manager.hpp"
#include "main.hpp"

#include "UserFilter.h"

namespace avs
{
	class WebCameraBase : public UserFilter
	{
	protected:
		int m_camera_index;

		void disconnect()
		{
			auto dev = WebCameraManager::instance()->get_device(m_camera_index);
			if (dev)
			{
				dev->close_acquisition();
				WebCameraManager::instance()->remove_device(m_camera_index);
			}
		}
		virtual void add_camera_params()
		{
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
		}
		virtual void read_inputs()
		{
			ReadInput(L"inCameraIndex", m_camera_index);
		}
	};
	class WebCameraBase_Input : public WebCameraBase
	{
	protected:
		int inWidth, inHeight, inFps, inInputQueueSize;
		std::shared_ptr<WebCamera> m_camera;
		void get_or_make_device()
		{
			auto mng = WebCameraManager::instance();
			auto existing = mng->get_device(m_camera_index);
			if (existing)
			{
				m_camera = existing;
			}
			m_camera = std::make_shared<WebCamera>(m_camera_index, inWidth, inHeight, inFps, inInputQueueSize);
			mng->add_device(m_camera);
		}
		virtual void add_camera_params() override
		{
			WebCameraBase::add_camera_params();
			AddInput(L"inWidth", L"Integer<0, INF>", L"", L"Image width");
			AddInput(L"inHeight", L"Integer<0, INF>", L"", L"Image height");
			AddInput(L"inFPS", L"Integer<0, 30>", L"", L"Desired FPS");
			AddInput(L"inInputQueueSize", L"Integer<1, 255>", L"128", L"Number of incoming frames that can be buffered before the application is able to process them");
		}
		virtual void read_inputs() override
		{
			WebCameraBase::read_inputs();
			ReadInput(L"inCameraIndex", m_camera_index);
			ReadInput(L"inWidth", inWidth);
			ReadInput(L"inHeight", inHeight);
			ReadInput(L"inFPS", inFps);
			ReadInput(L"inInputQueueSize", inInputQueueSize);
		}
		int Init() override
		{
			read_inputs();
			get_or_make_device();
			if (!m_camera->isRunning())
			{
				m_camera->start_acqisition();
			}
			return INIT_OK;
		}
	};
	// Example image processing filter
	class WebCameraStartAcquisition : public WebCameraBase_Input
	{
	private:
		// Non-trivial outputs must be defined as a filed to retain data after filter execution.
		avl::Image outImage;

	public:
		// Defines the inputs, the outputs and the filter metadata
		void Define() override
		{
			SetName(L"WebCameraStartAcquisition");
			SetTip(L"Starts acquisition of V4L webcamera");

			//					 Name						Type				Default		Tool-tip
			add_camera_params();
		}
		int Invoke() override
		{
			return INVOKE_LOOP;
		}
	};
	class WebCameraCloseAcquisition : public WebCameraBase
	{
	private:
		// Non-trivial outputs must be defined as a filed to retain data after filter execution.

	public:
		// Defines the inputs, the outputs and the filter metadata
		void Define() override
		{
			SetName(L"WebCameraStartAcquisition");
			SetTip(L"Starts acquisition of V4L webcamera");

			//					 Name						Type				Default		Tool-tip
			add_camera_params();
		}
		int Invoke() override
		{
			disconnect();
			return INVOKE_NORMAL;
		}
	};
	class WebCameraGrabImage : public WebCameraBase_Input
	{
	private:
		// Non-trivial outputs must be defined as a filed to retain data after filter execution.
		avl::Image outImage;

	public:
		// Defines the inputs, the outputs and the filter metadata
		void Define() override
		{
			SetName(L"WebCameraStartAcquisition");
			SetTip(L"Starts acquisition of V4L webcamera");

			//					 Name						Type				Default		Tool-tip
			add_camera_params();
			AddOutput("outImage", "Image", "Grabbed image");
			AddOutput("outFps", "Real", "Last reported fps");
		}
		int Invoke() override
		{
			while (!m_camera->can_grab())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				if (IsWorkCancelled())
				{
					outImage.Reset();
					WriteOutput("outImage", outImage);
					WriteOutput<float>("outFps", 0.0);
					return INVOKE_END;
				}
			}
			m_camera->grab_image(outImage);
			auto fps = (float)m_camera->get_fps();
			WriteOutput("outImage", outImage);
			WriteOutput<float>("outFps", fps);
			return INVOKE_LOOP;
		}
	};
	// 	struct MyRunningAverageState
	// {
	// 	bool isFirstTime;
	// 	float average;

	// 	MyRunningAverageState()
	// 		: isFirstTime(true)
	// 		, average(0.0f)
	// 	{
	// 	}
	// };
	// 	// Example filter with state
	// class MyRunningAverageFilter : public avs::UserFilter
	// {
	// private:
	// 	MyRunningAverageState state;
	// 	void MyRunningAverage( MyRunningAverageState& state, float value, float inertia, float& outAverage )
	// {
	// 	if (state.isFirstTime)
	// 	{
	// 		state.average = value;
	// 		state.isFirstTime = false;
	// 	}
	// 	else
	// 	{
	// 		state.average = state.average * inertia + value * (1.0f - inertia);
	// 	}

	// 	outAverage = state.average;
	// }
	// public:
	// 	void Define()
	// 	{
	// 		SetName		(L"MyRunningAverage");
	// 		SetCategory	(L"Basic::User Filters");
	// 		SetTip		(L"Calculates a geometrical average of previous average and current value.");
	// 		SetFilterType(avs::FilterType::LoopAccumulator);

	// 		//			 Name			Type		Default		Tool-tip
	// 		AddInput	(L"inValue",	L"Real",	L"",		L"");
	// 		AddInput	(L"inInertia",	L"Real",	L"0.9",		L"");
	// 		AddOutput	(L"outAverage",	L"Real",				L"");

	// 		// Describe a function that should be used as an equivalent of this filter.
	// 		AddAttribute(L"CodeGenInfo", L"MyFunctions::MyRunningAverage( state: MyFunctions::MyRunningAverageState; in: inValue; in: inInertia; out: outAverage ) Include(\"MyFunctions.h\")");
	// 	}

	// 	int Init()
	// 	{
	// 		UserFilter::Init();

	// 		state = MyRunningAverageState();

	// 		return avs::INIT_OK;
	// 	}

	// 	int Invoke()
	// 	{
	// 		float inValue;
	// 		float inInertia;
	// 		float outAverage;

	// 		// Get data from the inputs
	// 		ReadInput(L"inValue", inValue);
	// 		ReadInput(L"inInertia", inInertia);

	// 		// Use common function to process data
	// 		MyRunningAverage(state, inValue, inInertia, outAverage );

	// 		// Set output data
	// 		WriteOutput(L"outAverage", outAverage);

	// 		// Continue program
	// 		return avs::INVOKE_NORMAL;
	// 	}
	// };

	// Builds the filter factory
	class RegisterUserObjects
	{
	public:
		RegisterUserObjects()
		{
			// Remember to register every filter exported by the user filter library
			// RegisterFilter(CreateInstance<WebCameraStartAcquisition>);
			// RegisterFilter(CreateInstance<MyRunningAverageFilter>);
		}
	};

	static RegisterUserObjects registerUserObjects;
}
