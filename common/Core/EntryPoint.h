#pragma once
#include "Log.h"
#include "Api.h"
#include "Application.h"
extern void AppMain();

int main(int argc, char* argv[]) {

	Logger::Init();
	Core::SetAPI(Core::API::Vulkan);
	if (argc > 1) {
		if (!_strcmpi(argv[1], "gl")) {

			Core::SetAPI(Core::API::GL);
		}
		else {
			Core::SetAPI(Core::API::Vulkan);
		}
	}
	AppMain();
		
	return 0;
}
