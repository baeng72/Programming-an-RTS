#pragma once
#include <common.h>
#include "intpoint.h"
#include "object.h"
#include "camera.h"

constexpr float TILE_SIZE = 13.99f;
class CITY {
	friend class APPLICATION;
	std::vector<OBJECT> _objects;
	INTPOINT _size;
public:
	CITY();
	void Init(INTPOINT size);
	void Render(CAMERA* cam, mat4& viewProj, Renderer::DirectionalLight& light);
	vec3 GetCenter();
};
