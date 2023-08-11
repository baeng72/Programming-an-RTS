#pragma once
#include "../../../common/common.h"
#include "../../../common/Platform/Vulkan/VulkanEx.h"
#include "intpoint.h"
#include <glm/glm.hpp>

struct PARTICLE {
	glm::vec3 position;
	glm::vec4 color;
};

struct HEIGHTMAP {
	Renderer::RenderDevice*		_pDevice;
	std::unique_ptr<Renderer::Sprite>			_sprite;
	std::unique_ptr<Renderer::Texture>			_heightMapTexture;
	std::unique_ptr<Renderer::ParticleSwarm>	_particles;
	INTPOINT		_size;
	float			_maxHeight;		//the height of the highest peak
	float*			_pHeightMap;	//array with height values

	HEIGHTMAP(Renderer::RenderDevice* pdevice, INTPOINT size_);
	~HEIGHTMAP();
	bool LoadFromFile(const char* fileName);
	bool CreateParticles();
	void Render(glm::mat4&viewProj,glm::vec3&eyePos);
	glm::vec2 GetCenter() {
		return glm::vec2(_size.x / 2.f, _size.y / 2.f);
	}

};