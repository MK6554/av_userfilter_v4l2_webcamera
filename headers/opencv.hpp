#ifndef PLATFORM_UNIX
#ifndef UF_WIN_COMPAT
#define UF_WIN_COMPAT
#include <AVL_Lite.h>
namespace cv{
	struct Mat{};
	struct VideoCapture{};
	//int CAP_PROP_FPS;
}
namespace avl{
	void CVMatToAvlImage_Switched(const cv::Mat &mat, avl::Image& image);
}
#endif
#else
#include <opencv2/opencv.hpp>
#include <AVLConverters/AVL_OpenCV.h>
#endif
