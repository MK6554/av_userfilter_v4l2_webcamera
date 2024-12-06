#include <iostream>

#include <AVL_Lite.h>
#include <UserFilter.h>
#include <UserFilterLibrary.hxx>
#include "webcamera.hpp"
#include "webcamera_manager.hpp"
#include "main.hpp"
#include "debug_log.hpp"

namespace avs {

void WebCameraBase::disconnect() {
    auto dev = WebCameraManager::instance()->get_device(m_camera_index);
    if (dev) {
        dev->close_acquisition();
        WebCameraManager::instance()->remove_device(m_camera_index);
    }
}

void WebCameraBase::add_camera_params() {
    AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index");
}

void WebCameraBase::read_inputs() {
    ReadInput(L"inCameraIndex", m_camera_index);
}

void WebCameraBase_Input::get_or_make_device() {
    auto mng = WebCameraManager::instance();
    auto existing = mng->get_device(m_camera_index);
    if (existing) {
        m_camera = existing;
    }
    m_camera = std::make_shared<WebCamera>(m_camera_index, inWidth, inHeight, inFps, inInputQueueSize);
    mng->add_device(m_camera);
}

void WebCameraBase_Input::add_camera_params() {
    WebCameraBase::add_camera_params();
    AddInput(L"inWidth", L"Integer<0, INF>", L"", L"Image width");
    AddInput(L"inHeight", L"Integer<0, INF>", L"", L"Image height");
    AddInput(L"inFPS", L"Integer<0, 30>", L"", L"Desired FPS");
    AddInput(L"inInputQueueSize", L"Integer<1, 255>", L"128", L"Number of incoming frames that can be buffered before the application is able to process them");
}

void WebCameraBase_Input::read_inputs() {
    WebCameraBase::read_inputs();
    ReadInput(L"inWidth", inWidth);
    ReadInput(L"inHeight", inHeight);
    ReadInput(L"inFPS", inFps);
    ReadInput(L"inInputQueueSize", inInputQueueSize);
}

int WebCameraBase_Input::Init() {
    read_inputs();
    get_or_make_device();
    if (!m_camera->isRunning()) {
        m_camera->start_acqisition();
    }
    return INIT_OK;
}

void WebCameraStartAcquisition::Define() {
    SetName(L"WebCameraStartAcquisition");
    SetTip(L"Starts acquisition of V4L webcamera");
    add_camera_params();
}

int WebCameraStartAcquisition::Invoke() {
    return INVOKE_LOOP;
}

void WebCameraCloseAcquisition::Define() {
    SetName(L"WebCameraCloseAcquisition");
    SetTip(L"Closes acquisition of V4L webcamera");
    add_camera_params();
}

int WebCameraCloseAcquisition::Invoke() {
    disconnect();
    return INVOKE_NORMAL;
}

void WebCameraGrabImage::Define() {
    SetName(L"WebCameraGrabImage");
    SetTip(L"Grabs an image from the V4L camera, optionally starting the acquisition beforehand.");
    add_camera_params();
    AddOutput(L"outImage", L"Image", L"Grabbed image");
    AddOutput(L"outFPS", L"Real", L"Last reported fps");
}

int WebCameraGrabImage::Invoke() {
    while (!m_camera->can_grab()) {
        log("MY HEAD'S SPINNING");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        if (IsWorkCancelled()) {
            outImage.Reset();
            WriteOutput(L"outImage", outImage);
            WriteOutput<float>(L"outFPS", 0.0);
            return INVOKE_END;
        }
        WriteOutput(L"outImage", outImage);
        WriteOutput<float>(L"outFPS", 1.0);
        return INVOKE_LOOP;
    }
    m_camera->grab_image(outImage);
    auto fps = (float)m_camera->get_fps();
    WriteOutput(L"outImage", outImage);
    WriteOutput<float>(L"outFPS", fps);
    return INVOKE_LOOP;
}

RegisterUserObjects::RegisterUserObjects() {
    RegisterFilter(CreateInstance<WebCameraStartAcquisition>);
    RegisterFilter(CreateInstance<WebCameraGrabImage>);
    RegisterFilter(CreateInstance<WebCameraCloseAcquisition>);
}

static RegisterUserObjects registerUserObjects;

} // namespace avs
