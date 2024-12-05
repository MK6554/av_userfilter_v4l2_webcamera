#ifndef UF_WIN_COMPAT
#define UF_WIN_COMPAT
#ifndef PLATFORM_UNIX
#include <AVL_Lite.h>
namespace cv{
	struct Mat{};
	struct VideoCapture{};
	int CAP_PROP_FPS=0;
}
namespace avl{
	void CVMatToAvlImage_Switched(const cv::Mat &mat, avl::Image& image){
		avl::TestImage(avl::TestImageId::Baboon,image, atl::NIL);
	}
}
#else
#include <opencv2/opencv.hpp>
#include <AVLConverters/AVL_OpenCV.h>
#endif
#endif