#include "webcamera_manager.hpp"

WebCameraManager::WebCameraManager() {}
WebCameraManager::~WebCameraManager() {}

void WebCameraManager::add_device(std::shared_ptr<WebCamera> camera)
{
    if(get_device(camera->camera_index())){
        throw -12;
    }
    devices[camera->camera_index()]=camera;
}

std::shared_ptr<WebCamera> WebCameraManager::get_device(int id)
{
    auto found = devices.find(id);
    if (found != devices.end())
    {
        return found->second;
    }
    return std::shared_ptr<WebCamera>(nullptr);
}
