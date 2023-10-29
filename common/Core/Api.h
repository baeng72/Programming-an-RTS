#pragma once
#include "defines.h"
namespace Core {
	enum class API{GL,Vulkan};
	API GetAPI();
	void SetAPI(API api);
	extern mat4 (*perspective)(float fov, float width, float height, float near, float far);
	extern mat4 (*orthoWH)(float width, float height, float near, float far);
	extern mat4 (*orthoLTRB)(float left, float top, float right, float bottom, float near, float far);
	extern vec3(*project)(const vec3& obj, const mat4& world, const mat4& mvp, const vec4& viewport);
}