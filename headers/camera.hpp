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
    double get_property(int property_id)const;
    bool isRunning() const {return true;}
    WebCamera(int cameraIndex, int width, int height, int framerate);
    double get_fps()const{return queue.get_fps();}
    size_t received_frames()const{return m_received_frames.load();}
    ~WebCamera();
    private:
    std::atomic<size_t> m_received_frames;
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