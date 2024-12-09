#include "webcamera.hpp"
#include "webcamera_manager.hpp"
int main(int argc, char *argv[]) {
  auto s = std::make_shared<WebCamera>(0, 2592/1, 1944/1, 30);
  WebCameraManager::instance()->add_device(s);
  auto m_camera = WebCameraManager::instance()->get_device(0);
  if (!m_camera->isRunning()) {
    m_camera->start_acqisition();
  }
  while (true) {
    avl::Image img;
    // std::cout << "\r" << m_camera->get_fps() <<"   "<<  std::flush;
    if (m_camera->can_grab()) {
      m_camera->grab_image(img);
      std::cout << "\r"
                << "FPS: " << std::fixed << std::setprecision(2)
                << m_camera->get_fps() 
                 << ", size: "<<img.Width()<<" x "<<img.Height()
                << std::flush;
    }
  }
}
