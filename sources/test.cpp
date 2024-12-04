#include "camera.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "cpuStats.hpp"
#include "debug_log.hpp"
int main(int argc, char *argv[]) {
  std::cout << "Henlo\n";
  WebCamera cammy(0,2592,1944,30);
  ProcessUsage calc(getpid());
  cammy.start_acqisition();
  log("cammy!");
  avl::Image img;
  int  i =0;
  while (true) {
    i++;
    calc.measure();
    if (cammy.grab_image(img)) {
      std::cout << "\r" << "FPS: " << std::fixed <<std::setprecision (2) <<cammy.get_fps() 
      << " (target: "<< std::fixed <<std::setprecision (0)<<cammy.get_property(cv::CAP_PROP_FPS)<<")"
      <<", received frames: "<<cammy.received_frames() 
      <<", CPU: " <<std::fixed <<std::setprecision (2)<<calc.getMeanUsage()<<"%"
      <<std::flush;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}