#include <iostream>

#include <AVL_Lite.h>
#include "main.hpp"

#include "UserFilter.h"

namespace avs
{
// 	// Example image processing filter
// 	class CustomThreshold : public UserFilter
// 	{
// 	private:
// 		// Non-trivial outputs must be defined as a filed to retain data after filter execution. 
// 		avl::Image outImage;

// 	public:
// 		// Defines the inputs, the outputs and the filter metadata
// 		void Define() override
// 		{
// 			SetName		(L"CustomThreshold");
// 			SetCategory	(L"Image::Image Thresholding");
// 			SetImage	(L"CustomThreshold_16.png");	
// 			SetImageBig	(L"CustomThreshold_48.png");
// 			SetTip		(L"Binarizes 8-bit images");

// 			//					 Name						Type				Default		Tool-tip
// 			AddInput			(L"inImage",				L"Image",			L"",		L"Input image"    );
// 			AddInput			(L"inThreshold",			L"Integer<0, 255>",	L"128",		L"Threshold value");
// 			AddOutput			(L"outImage",				L"Image",						L"Output image"   );
// 		}

// 		// Computes output from input data
// 		int Invoke() override
// 		{
// 			// Get data from the inputs
// 			avl::Image inImage;
// 			int inThreshold;
// 			//avs::LogInfo("Andrzej Duda czyni cuda !!!!!!!!!!");

// 			ReadInput(L"inImage", inImage);
// 			ReadInput(L"inThreshold", inThreshold);

// 			if (inImage.Type() != avl::PlainType::UInt8)
// 				throw atl::DomainError("Only uint8 pixel type are supported.");

// 			// Get image properties
// 			int height = inImage.Height();

// 			// Prepare output image in this same format as input
// 			outImage.Reset(inImage, atl::NIL);

// 			// Enumerate each row
// 			for (int y = 0; y < height; ++y)
// 			{
// 				// Get row pointers
// 				const atl::uint8* p = inImage.RowBegin<atl::uint8>(y);
// 				const atl::uint8* e = inImage.RowEnd<atl::uint8>(y);
// 				atl::uint8*       q = outImage.RowBegin<atl::uint8>(y);

// 				// Loop over the pixel components
// 				while (p < e)
// 				{
// 					(*q++) = (*p++) < inThreshold ? 0 : 255;
// 				}
// 			}

// 			// Set output data
// 			WriteOutput(L"outImage", outImage);

// 			// Continue program
// 			return INVOKE_NORMAL;
// 		}
// 	};
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
			// RegisterFilter(CreateInstance<CustomThreshold>);
			// RegisterFilter(CreateInstance<MyRunningAverageFilter>);
		}
	};

	static RegisterUserObjects registerUserObjects;
}
