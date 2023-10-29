#include "functions.h"

INTPOINT GetScreenPos(vec3 pos, mat4& matViewProj, vec4& viewport) {
	//vec3 screenPos = glm::project(pos, glm::mat4(1.f), matViewProj, viewport);
	vec3 screenPos = Core::project(pos, glm::mat4(1.f), matViewProj, viewport);
	return INTPOINT((int)screenPos.x, (int)screenPos.y);
}

INTPOINT GetScreenPos(vec3 pos, mat4& matProj, mat4& matView, vec4& viewport)
{
	mat4 matVP = matProj * matView;
	return GetScreenPos(pos,matVP,viewport);
}
