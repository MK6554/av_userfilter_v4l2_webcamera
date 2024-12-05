#include "opencv.hpp"
#ifndef PLATFORM_UNIX
void avl:: CVMatToAvlImage_Switched(const cv::Mat &mat, avl::Image& image){
	avl::TestImage(avl::TestImageId::Baboon,image, atl::NIL);
}
#endif
