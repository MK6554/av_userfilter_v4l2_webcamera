#include "main.hpp"
#include "debug_log.hpp"
#include "webcamera.hpp"
#include "webcamera_manager.hpp"
#include <AVL_Lite.h>
#include <UserFilter.h>
#include <UserFilterLibrary.hxx>
#include <iostream>

namespace avs {

#define THROW_NOT_CONNECTED                                                    \
  throw atl::IoError("Attempted to reuse a device that does not exist.")

void WebCameraBase::disconnect() {
  auto dev = WebCameraManager::instance()->get_device(m_camera_index);
  if (dev) {
    dev->close_acquisition();
    WebCameraManager::instance()->remove_device(m_camera_index);
  }
}

void WebCameraBase::add_camera_params() {
  AddInput(L"inCameraIndex", L"Integer<0, INF>", L"0", L"Device index.");
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
  m_camera = std::make_shared<WebCamera>(m_camera_index, inWidth, inHeight,
                                         inFps, inExposure);
  mng->add_device(m_camera);
}

void WebCameraBase_Input::add_camera_params() {
  WebCameraBase::add_camera_params();
  AddInput(L"inWidth", L"Integer<1, INF>*", L"",
           L"Image width. Leave auto to use maximum available height.");
  AddInput(L"inHeight", L"Integer<1, INF>*", L"",
           L"Image height. Leave auto to use maximum available height.");
  AddInput(L"inFPS", L"Integer<0, INF>", L"30",
           L"FPS limit. Camera will be throttled down to this framerate.");
  AddInput(L"inExposure", L"Integer<10, INF>", L"100",
           L"Desired exposure time in milliseconds.");
}

void WebCameraBase_Input::read_inputs() {
  WebCameraBase::read_inputs();
  atl::Optional<int> opt_width, opt_height;
  ReadInput(L"inWidth", opt_width);
  ReadInput(L"inHeight", opt_height);
  inHeight = opt_height.GetValueOr(-1);
  inWidth = opt_width.GetValueOr(-1);
  ReadInput(L"inFPS", inFps);
  ReadInput(L"inExposure", inExposure);
}

int WebCameraBase_Input::Init() {
  read_inputs();
  get_or_make_device();
  if (!m_camera->is_running()) {
    m_camera->start_acquisition();
  }
  return INIT_OK;
}

void WebCameraStartAcquisition::Define() {
  SetName(L"WebCamera_StartAcquisition");
  SetTip(L"Starts acquisition of V4L webcamera");
  add_camera_params();
}

int WebCameraStartAcquisition::Invoke() { return INVOKE_LOOP; }

void WebCameraCloseAcquisition::Define() {
  SetName(L"WebCamera_loseAcquisition");
  SetTip(L"Closes acquisition of V4L webcamera");
  add_camera_params();
}

int WebCameraCloseAcquisition::Invoke() {
  disconnect();
  return INVOKE_NORMAL;
}

void WebCameraGrabImage::Define() {
  SetName(L"WebCamera_GrabImage");
  SetTip(L"Grabs an image from the V4L camera, optionally starting the "
         L"acquisition beforehand.");
  add_camera_params();
  AddOutput(L"outImage", L"Image", L"Grabbed image.");
  AddOutput(L"outFPS", L"Real", L"Last reported FPS.");
}

int WebCameraGrabImage::Invoke() {
  while (!m_camera->can_grab()) {
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
/*
SET PARAMETER
*/
void WebCameraSetParameter::Define() {
  SetName(L"WebCamera_SetParameter");
  SetTip(L"Set a parameter from the camera.");
  add_camera_params();
  AddInput(L"inParameterId", L"Integer<0, INF>", L"0", L"Parameter ID.");
  AddInput(L"inParameterValue", L"Double", L"0", L"Parameter value.");
  AddOutput(L"outSuccess", L"Bool", L"Status");
  AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<Generic>");
}
int WebCameraSetParameter::Invoke() {
  read_inputs();
  int id;
  double val;
  ReadInput(L"inParameterId", id);
  ReadInput(L"inParameterValue", val);
  switch (id) {
  case cv::CAP_PROP_EXPOSURE:
    throw atl::DomainError("To set exposure use the dedicated filter.");
  case cv::CAP_PROP_FPS:
    throw atl::DomainError("To limit frame rate use the dedicated filter.");
  }
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  auto res = cam->set_property(id, val);
  WriteOutput(L"outSuccess", res);
  return INVOKE_NORMAL;
}
/*
SET FRAME RATE LIMIT
*/
void WebCameraSetFrameRateLimit::Define() {
  SetName(L"WebCamera_SetFrameRateLimit");
  SetTip(L"Set new frame rate limit.");
  add_camera_params();
  AddInput(L"inFpsLimit", L"Integer<1,INF>", L"30", L"Fps limit.");
  AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<FrameRate>");
}
int WebCameraSetFrameRateLimit ::Invoke() {
  read_inputs();
  int val;
  ReadInput(L"inFpsLimit", val);
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  cam->set_max_framerate(val);
  return INVOKE_NORMAL;
}
/*
SET EXPOSURE
*/
void WebCameraSetExposure::Define() {
  SetName(L"WebCamera_SetExposure");
  SetTip(L"Set new exposure to camera (if not on auto exposure).");
  add_camera_params();
  AddInput(L"inExposure_ms", L"Double", L"0", L"Parameter value");
  AddAttribute(L"FilterGroup", L"UfWebCamera_SetParameter<Exposure> default ## "
                               L"modify web camera's parameters.");
}
int WebCameraSetExposure::Invoke() {
  read_inputs();
  int val;
  ReadInput(L"inExposure_ms", val);
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  cam->set_exposure(val);
  return INVOKE_NORMAL;
}
/*
GET PARAMETER
*/
void WebCameraGetParameter::Define() {
  SetName(L"WebCamera_GetParameter");
  SetTip(L"Get a parameter from the camera.");
  add_camera_params();
  AddInput(L"inParameterId", L"Integer<0, INF>", L"0", L"Parameter ID");
  AddOutput(L"outValue", L"Double", L"Parameter value");
  AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<Generic>");
}
int WebCameraGetParameter::Invoke() {
  read_inputs();
  int id;
  ReadInput(L"inParameterId", id);
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  switch (id) {
  case cv::CAP_PROP_EXPOSURE:
    WriteOutput(L"outValue", (double)cam->exposure().count());
    break;
  case cv::CAP_PROP_FPS:
    WriteOutput(L"outValue", (double)cam->max_framerate());
    break;
  default:
    auto res = cam->get_property(id);
    WriteOutput(L"outValue", res);
    break;
  }
  return INVOKE_NORMAL;
}
/*
GET FRAME RATE
*/
void WebCameraGetFrameRateLimit::Define() {
  SetName(L"WebCamera_GetFrameRateLimit");
  SetTip(L"Gets current value of frame rate limit.");
  add_camera_params();
  AddOutput(L"outFpsLimit", L"Integer", L"Fps limit.");
  AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<FrameRate>");
}
int WebCameraGetFrameRateLimit::Invoke() {
  read_inputs();
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  WriteOutput(L"outFpsLimit", (double)cam->max_framerate());
  return INVOKE_NORMAL;
}
/*
GET EXPOSURE
*/
void WebCameraGetExposure::Define() {
  SetName(L"WebCamera_GetExposure");
  SetTip(L"Gets current value of exposure (manually set only).");
  add_camera_params();
  AddOutput(L"outExposure", L"Integer", L"Exposure time in milliseconds");
  AddAttribute(L"FilterGroup", L"UfWebCamera_GetParameter<Exposure> default ## "
                               L"read web camera's parameters.");
}
int WebCameraGetExposure::Invoke() {
  read_inputs();
  auto cam = WebCameraManager::instance()->get_device(m_camera_index);
  if (!cam) {
    THROW_NOT_CONNECTED;
  }
  WriteOutput(L"outExposure", (int)cam->exposure().count());
  return INVOKE_NORMAL;
}

RegisterUserObjects::RegisterUserObjects() {
  RegisterFilter(CreateInstance<WebCameraStartAcquisition>);
  RegisterFilter(CreateInstance<WebCameraGrabImage>);
  RegisterFilter(CreateInstance<WebCameraCloseAcquisition>);
  RegisterFilter(CreateInstance<WebCameraGetParameter>);
  RegisterFilter(CreateInstance<WebCameraGetFrameRateLimit>);
  RegisterFilter(CreateInstance<WebCameraGetExposure>);
  RegisterFilter(CreateInstance<WebCameraSetParameter>);
  RegisterFilter(CreateInstance<WebCameraSetFrameRateLimit>);
  RegisterFilter(CreateInstance<WebCameraSetExposure>);
}

static RegisterUserObjects registerUserObjects;

} // namespace avs
