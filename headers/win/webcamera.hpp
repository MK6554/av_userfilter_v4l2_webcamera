#ifndef UF_WEB_CAMERA
#define UF_WEB_CAMERA
#include <AVL_Lite.h>
#include <chrono>
#include <iostream>
namespace cv{
  const int CAP_PROP_EXPOSURE = 15;
  const int CAP_PROP_FPS = 5;
}

class WebCamera {
public:
  void start_acquisition(){}
  bool grab_image(avl::Image &image){
    avl::Image canvas=singleton_image;
    auto style = avl::DrawingStyle();
    avl::DrawString(canvas,std::to_string(counter).c_str(),avl::Location(0,0),atl::NIL,avl::Anchor2D::TopLeft,avl::Pixel(150,95,212),style,48,0,avl::Pixel(139,212,80));
    image = canvas;
    image.MakeDataOwn();
    counter++;
    return true;
  }
  void close_acquisition(){}
  WebCamera(int cameraIndex, int width, int height, int framerate, long exposure_ms){
    avl::TestImage(avl::TestImageId::Baboon,singleton_image,atl::NIL);
    counter= 0 ;
  }
  ~WebCamera(){}
  bool set_property(int property_id, double value){return true;}
  double get_property(int property_id) const{return 37.21;}

  int camera_index() const { return 0; }
  int max_framerate() const { return 30; }
  int width() const { return singleton_image.Width(); }
  int height() const { return singleton_image.Height(); }
  bool can_grab() const {return true;}
  bool is_running() const { return true; }
  double get_fps() const { return 24; }
  std::chrono::milliseconds exposure() const { return std::chrono::milliseconds(250); }

  void set_max_framerate(int new_max){}
  void set_exposure(long time_ms){}
  void set_exposure(std::chrono::milliseconds millis){}
private:
  avl::Image singleton_image;
  int counter;
};
// void list_supported_framerates(std::string const &devicePath){std::cout << "ITS JUST A MOCKUP, DONT USE ON WINDOWS\n";}

#endif