#pragma once

namespace Core {
	enum class API{GL,Vulkan};
	API GetAPI();
	void SetAPI(API api);
}