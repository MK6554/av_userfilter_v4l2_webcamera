/****************************************************************************************
*
* EXAMPLE USER FILTERS
*
* To create your own filter, you have to create a class derived from the UserFilter
* class and override at least 'Define' and 'Invoke' methods.
*
* All user filter classes then have to be registered in
* the constructor of RegisterUserObjects class (at the end of the file).
*
* See the examples below for more details.
*
* More information:
* http://docs.adaptive-vision.com/current/studio/extensibility/CreatingUserFilters.html
*
* For AVL Lite documentation visit:
* http://docs.adaptive-vision.com/current/avl_lite/
*
****************************************************************************************/

#include "winUserFilterMockUp.h"
#include "UserFilter.h"
#include "AVL_Lite.h"
#include "ThirdPartySdk_Lite.h"

#include "UserFilterLibrary.hxx"

namespace avs
{

	// Example image processing filter
	class WebCameraGrabImage : public UserFilter
	{
	private:
		avl::Image outImage;

	public:
		/// Defines the inputs, the outputs and the metadata
		void Define() override
		{
			SetName(L"WebCameraGrabImage");
			SetTip(L"Grabs an image from the V4L camera, optionally starting the acquisition beforehand.");
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
			AddInput(L"inWidth", L"Integer<0, INF>", L"", L"Image width");
			AddInput(L"inHeight", L"Integer<0, INF>", L"", L"Image height");
			AddInput(L"inFPS", L"Integer<0, 30>", L"", L"Desired FPS");
			AddInput(L"inInputQueueSize", L"Integer<1, 255>", L"128", L"Number of incoming frames that can be buffered before the application is able to process them");
			AddOutput(L"outImage", L"Image", L"Grabbed image");
			AddOutput(L"outFPS", L"Real", L"Last reported fps");
		}

		/// Computes output from input data
		int Invoke() override
		{
			avl::TestImage(avl::TestImageId::Baboon, outImage);
			WriteOutput(L"outImage", outImage);
			WriteOutput<float>(L"outFPS", 1.0);
			// Continue program
			return INVOKE_LOOP;
		}
	};


	// Example region processing filter
	class WebCameraStartAcquisition : public UserFilter
	{
	public:
		void Define() override
		{
			SetName(L"WebCameraStartAcquisition");
			SetTip(L"Starts acquisition of V4L webcamera");
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
			AddInput(L"inWidth", L"Integer<0, INF>", L"", L"Image width");
			AddInput(L"inHeight", L"Integer<0, INF>", L"", L"Image height");
			AddInput(L"inFPS", L"Integer<0, 30>", L"", L"Desired FPS");
			AddInput(L"inInputQueueSize", L"Integer<1, 255>", L"128", L"Number of incoming frames that can be buffered before the application is able to process them");
		}

		int Invoke() override
		{
			// Continue program
			return INVOKE_NORMAL;
		}
	};


	// Example profile creation filter
	class WebCameraCloseAcquisition : public UserFilter
	{
	private:
		avl::Profile outProfile;

	public:
		void Define() override
		{
			SetName(L"WebCameraCloseAcquisition");
			SetTip(L"Closes acquisition of V4L webcamera");
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
		}

		int Invoke() override
		{
			// Continue program
			return INVOKE_NORMAL;
		}
	};

	class WebCameraGetParameter : public UserFilter
	{
	private:
	public:
		void Define() override
		{
			SetName(L"WebCameraGetParameter");
			SetTip(L"Get a parameter from the camera.");
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
			AddInput(L"inParameterId", L"Integer<0, INF>", L"0", L"Parameter ID");
			AddOutput(L"outValue", L"Double", L"Parameter value");
		}

		int Invoke() override
		{
			// Continue program
			WriteOutput<double>(L"outValue", 12.5);
			return INVOKE_NORMAL;
		}
	};

	class WebCameraSetParameter : public UserFilter
	{
	private:

	public:
		void Define() override
		{
			SetName(L"WebCameraGetParameter");
			SetTip(L"Get a parameter from the camera.");
			AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
			AddInput(L"inParameterId", L"Integer<0, INF>", L"0", L"Parameter ID");
			AddInput(L"inParameterValue", L"Double", L"0", L"Parameter value");
			AddOutput(L"outSuccess", L"Bool", L"Status");
		}

		int Invoke() override
		{
			WriteOutput<double>(L"outSuccess", true);
			// Continue program
			return INVOKE_NORMAL;
		}
	};



	/// Builds the filter factory
	class RegisterUserObjects
	{
	public:
		RegisterUserObjects()
		{
			// Remember to register every filter exported by the user filter library
			RegisterFilter(CreateInstance<WebCameraCloseAcquisition>);
			RegisterFilter(CreateInstance<WebCameraGrabImage>);
			RegisterFilter(CreateInstance<WebCameraStartAcquisition>);
			RegisterFilter(CreateInstance<WebCameraGetParameter>);
			RegisterFilter(CreateInstance<WebCameraSetParameter>);
		}
	};

	static RegisterUserObjects registerUserObjects;

}
