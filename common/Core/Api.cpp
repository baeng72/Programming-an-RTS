#include "Api.h"
#include <iostream>

namespace Core {
	API gAPI = API::Vulkan;
	mat4(*perspective)(float fov, float width, float height, float near, float far);
	mat4(*orthoWH)(float width, float height, float near, float far);
	mat4(*orthoLTRB)(float left, float top, float right, float bottom, float near, float far);
	vec3(*project)(const vec3& obj, const mat4& world, const mat4& mvp, const vec4& viewport);
	API GetAPI() {
		return gAPI;
	}
	void SetAPI(API api) {
		gAPI = api;
		switch (api) {
		case API::Vulkan:
			//std::cout << "Vulkan API Selected" << std::endl;
			perspective = vulkPerspectiveLH;// glm::perspectiveFovLH_ZO;
			orthoWH = vulkOrthoLH;
			orthoLTRB = vulkOrthoLH;
			project = vulkProject;
			break;
		case API::GL:
			//std::cout << "OpenGL API Selected" << std::endl;
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__
			perspective = vulkPerspectiveLH;// glm::perspectiveFovLH_ZO;
			orthoWH = vulkOrthoLH;
			orthoLTRB = vulkOrthoLH;
			project = vulkProject;
#else
			perspective = glPerspectiveLH;
			orthoWH = glOrthoLH;
			orthoLTRB = glOrthoLH;
			project = glProject;
#endif
			break;
		}
	}
}