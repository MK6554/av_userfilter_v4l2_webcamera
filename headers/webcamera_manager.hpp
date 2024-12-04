#ifndef UF_WEBCAM_MANAG
#define UF_WEBCAM_MANAG
#include <unordered_map>
#include "webcamera.hpp"
class WebCameraManager{

	std::unordered_map<int,std::shared_ptr<WebCamera>> devices;
	WebCameraManager();
	~WebCameraManager();
	public:
    static WebCameraManager* instance()
	{
		static WebCameraManager singleton;
		return &singleton;
	}
	void add_device(std::shared_ptr<WebCamera> camera);
	std::shared_ptr<WebCamera> get_device(int id);
	bool remove_device(int id);
};
#endif