#ifndef UF_WEB_CAMERA
#define UF_WEB_CAMERA
#include "paraqueue.hpp"
#include <atomic>
#include <thread>
class WebCamera{
     public:
    void start_acqisition();
    bool grab_image(avl::Image& image);
    bool grab_image_sync();
    void close_acqisition();
    bool isRunning() const {return true;}
    WebCamera(int cameraIndex, int width, int height, int framerate);
    ~WebCamera();
    private:
    int width, height, framerate;
    int cameraIndex;
    cv::VideoCapture cap;
    std::thread capThread;
    std::atomic<bool> running;
    ParaQueue queue;
    void captureLoop();
};
void list_supported_framerates(std::string const & devicePath);

#endif