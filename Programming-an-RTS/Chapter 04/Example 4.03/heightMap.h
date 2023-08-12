#pragma once
#include "../../../common/common.h"
#include "../../../common/Platform/Vulkan/VulkanEx.h"
#include "intpoint.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

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
	Rect			_selRect;
	HEIGHTMAP(Renderer::RenderDevice* pdevice, INTPOINT size_);
	~HEIGHTMAP();
	bool LoadFromFile(const char* fileName);
	bool CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves);
	bool CreateParticles();
	void Render(glm::mat4&viewProj,glm::vec3&eyePos);
	glm::vec2 GetCenter() {
		return glm::vec2(_size.x / 2.f, _size.y / 2.f);
	}
	void	MoveRect(int dir);
	void RaiseTerrain(Rect& r, float f);
	void SmoothTerrain();

};