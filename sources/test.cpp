#include "webcamera.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "cpu_stats.hpp"
#include "debug_log.hpp"
int main(int argc, char *argv[]) {
  WebCamera cammy(0,2592,1944,30,100);
  CpuStats calc;
  cammy.start_acqisition();
  avl::Image img;
  int  i =0;
  while (true) {
    calc.measure();
    i++;
    if (cammy.grab_image(img)) {
      std::cout << "\r" << "FPS: " << std::fixed <<std::setprecision (2) <<cammy.get_fps() 
      << ", size: "<<img.Width()<<" x "<<img.Height()
      // << " (target: "<< std::fixed <<std::setprecision (0)<<cammy.get_property(5)<<")"
      <<", enqueued frames: "<<cammy.enqueued_frames() 
      <<", CPU: " <<std::fixed <<std::setprecision (2)<<calc.getMeanUsage()<<"%"
      <<std::flush;
    }
     std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}
