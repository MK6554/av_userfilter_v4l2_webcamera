#include "cpu_stats.hpp"
#include "debug_log.hpp"
#include "webcamera.hpp"
#include "webcamera_manager.hpp"
#include <chrono>
#include <iostream>
#include <thread>
int main(int argc, char *argv[]) {
  auto s = std::make_shared<WebCamera>(0, 2592, 1944, 30, 100);
  WebCameraManager::instance()->add_device(s);
  auto m_camera = WebCameraManager::instance()->get_device(0);
  if (!m_camera->isRunning()) {
    m_camera->start_acqisition();
  }
  while(true){
    avl::Image img;
    if(m_camera->can_grab()){
      m_camera->grab_image(img);
      std::cout << "\r" << "FPS: " << std::fixed <<std::setprecision (2)
        <<m_camera->get_fps()
        <<std::flush;
    }
  }

  //   WebCamera cammy(0,2592,1944,30,100);
  //   CpuStats calc;
  //   cammy.start_acqisition();
  //   avl::Image img;
  //   int  i =0;
  //   while (true) {
  //     calc.measure();
  //     i++;
  //     if (cammy.grab_image(img)) {
  //       std::cout << "\r" << "FPS: " << std::fixed <<std::setprecision (2)
  //       <<cammy.get_fps()
  //       << ", size: "<<img.Width()<<" x "<<img.Height()
  //       // << " (target: "<< std::fixed <<std::setprecision
  //       (0)<<cammy.get_property(5)<<")"
  //       <<", enqueued frames: "<<cammy.enqueued_frames()
  //       <<", CPU: " <<std::fixed <<std::setprecision
  //       (2)<<calc.getMeanUsage()<<"%"
  //       <<std::flush;
  //     }
  //      std::this_thread::sleep_for(std::chrono::milliseconds(250));
  //   }
}
