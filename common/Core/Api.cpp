#include "Api.h"
#include <iostream>

namespace Core {
	API gAPI = API::Vulkan;
	API GetAPI() {
		return gAPI;
	}
	void SetAPI(API api) {
		gAPI = api;
		switch (api) {
		case API::Vulkan:
			std::cout << "Vulkan API Selected" << std::endl;
			break;
		case API::GL:
			std::cout << "OpenGL API Selected" << std::endl;
			break;
		}
	}
}