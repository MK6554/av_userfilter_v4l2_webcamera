#include "camera.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include "debug_log.hpp"
int main(int argc, char *argv[]) {
  std::cout << "Henlo\n";
  WebCamera cammy(0,2592,1944,30);
  cammy.start_acqisition();
  log("cammy!");
  avl::Image img;
  int  i =0;
  while (true) {
    i++;
    if (cammy.grab_image(img)) {
      std::cout << "\r" << "FPS: " << std::setprecision (4) <<cammy.get_fps() <<", received frames: "<<cammy.received_frames() << "      \033[?25l"<<std::flush;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }
}