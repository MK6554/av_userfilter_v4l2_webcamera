#ifndef MAIN
#define MAIN
#include <AVL_Lite.h>
#include <UserFilter.h>
// #include <UserFilterLibrary.hxx>
#include "webcamera.hpp"
namespace avs {

class WebCameraBase : public UserFilter {
protected:
    int m_camera_index;
    void disconnect();
    virtual void add_camera_params();
    virtual void read_inputs();
};

class WebCameraBase_Input : public WebCameraBase {
protected:
    int inWidth, inHeight, inFps, inInputQueueSize;
    std::shared_ptr<WebCamera> m_camera;
    void get_or_make_device();
    virtual void add_camera_params() override;
    virtual void read_inputs() override;
    int Init() override;
};

class WebCameraStartAcquisition : public WebCameraBase_Input {
private:
    avl::Image outImage;

public:
    void Define() override;
    int Invoke() override;
};

class WebCameraCloseAcquisition : public WebCameraBase {
public:
    void Define() override;
    int Invoke() override;
};

class WebCameraGrabImage : public WebCameraBase_Input {
private:
    avl::Image outImage;

public:
    void Define() override;
    int Invoke() override;
};

class RegisterUserObjects {
public:
    RegisterUserObjects();
};

} // namespace avs

#endif /* MAIN */
