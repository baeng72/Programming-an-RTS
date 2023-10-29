#include "functions.h"

INTPOINT GetScreenPos(vec3 pos, mat4& matViewProj, vec4& viewport) {
	vec3 screenPos;
	if (Core::GetAPI() == Core::API::Vulkan) {
		screenPos = Core::project(pos, mat4(1.f), matViewProj, viewport);
		//screenPos = glm::projectZO(pos, glm::mat4(1.f), matViewProj, viewport);
	}
	else {
		//screenPos = glm::projectNO(pos, glm::mat4(1.f), matViewProj, viewport);
		//float py = 2.f * screenPos.y / (float)viewport.w - 1.f;
		//py *= -1;
		//screenPos.y  = viewport.w * (py + 1.f) * 0.5f;
		screenPos = Core::project(pos, mat4(1.f), matViewProj, viewport);
	}
	
	return INTPOINT((int)screenPos.x, (int)screenPos.y);
}

INTPOINT GetScreenPos(vec3 pos, mat4& matProj, mat4& matView, vec4& viewport)
{
	mat4 matVP = matProj * matView;
	return GetScreenPos(pos,matVP,viewport);
}
