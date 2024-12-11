#include "webcamera.hpp"
#include "webcamera_manager.hpp"
#include <iostream>
void run_loop()
{
  auto s = std::make_shared<WebCamera>(0, 2592 / 1, 1944 / 1, 30, 200);
  WebCameraManager::instance()->add_device(s);
  auto m_camera = WebCameraManager::instance()->get_device(0);
  if (!m_camera->is_running())
  {
    m_camera->start_acquisition();
  }
  while (true)
  {
    avl::Image img;
    // std::cout << "\r" << m_camera->get_fps() <<"   "<<  std::flush;
    if (m_camera->can_grab())
    {
      m_camera->grab_image(img);
      std::cout << "\r"
                << "FPS: " << std::fixed << std::setprecision(2)
                << m_camera->get_fps() << ", size: " << img.Width() << " x "
                << img.Height() << std::flush;
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc > 1 && argv[1] == "--loop")
  {
    run_loop();
  }
  int dev_num = 0;
  if (argc > 1)
  {
    dev_num = strtol(argv[1], nullptr, 16);
  }
#ifdef PLATFORM_UNIX
  list_supported_framerates("/dev/video" + std::to_string(dev_num));
#endif
}
