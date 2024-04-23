#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"

namespace Renderer {
	struct DirectionalLight {
		using color = glm::vec4;
		color 		diffuse;
		color 		specular;
		color 		ambient;
		glm::vec3	direction;
		float		padding{ 0.f };
		DirectionalLight(){
			diffuse = specular = ambient = color(0.f);
			direction = glm::vec3(0.f);			
		}		
	};
}
