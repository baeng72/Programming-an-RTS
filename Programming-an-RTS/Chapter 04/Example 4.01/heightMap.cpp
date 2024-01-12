#pragma once
#include "heightMap.h"
#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>


HEIGHTMAP::HEIGHTMAP(Renderer::RenderDevice* pdevice, INTPOINT size_)
	:_size(size_),_pDevice(pdevice),_maxHeight(15.f)
{
	_pHeightMap = nullptr;
}

HEIGHTMAP::~HEIGHTMAP()
{
	delete[] _pHeightMap;
	_sprite.reset();
	_heightMapTexture.reset();
}

bool HEIGHTMAP::LoadFromFile(const char* fileName)
{
	_sprite.reset(Renderer::Sprite::Create(_pDevice));
	glm::vec2 size(_size.x, _size.y);
	_heightMapTexture.reset(Renderer::Texture::Create(_pDevice, fileName, size));
	int texWidth, texHeight, texChannels;
	stbi_uc* texPixels = stbi_load(fileName, &texWidth, &texHeight, &texChannels, STBI_grey);// STBI_rgb_alpha);
	assert(texPixels);

	if (_size.x != texWidth || _size.y != texHeight) {
		//need to resize, probably better ways to do this, but meh
		stbi_uc* newTexPixels = (stbi_uc*)malloc(_size.x * _size.y * 1);

		stbir_resize_uint8_linear(texPixels, texWidth, texHeight, texWidth,
			newTexPixels, _size.x, _size.y, 0,
			(stbir_pixel_layout)1);

		stbi_image_free(texPixels);
		texPixels = newTexPixels;
		texWidth = _size.x;
		texHeight = _size.y;
	}
	if (_pHeightMap)
		delete[]_pHeightMap;
	_pHeightMap = new float[_size.x * _size.y];
	//extract height values
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			unsigned char b = texPixels[y * texWidth + x];
			float h = ((float)b / 255.0f) * _maxHeight;
			_pHeightMap[(_size.x - x - 1) + y * _size.x] = h;
		}
	}
	stbi_image_free(texPixels);
	CreateParticles();
	return true;
}

bool HEIGHTMAP::CreateParticles()
{
	std::vector<Renderer::ParticleVertex> vertices;
	for (int32_t y = 0; y < _size.y; ++y) {
		for (int32_t x = 0; x < _size.x; ++x) {
			float fHeightRaw = _pHeightMap[x + y * _size.x];
			float fHeight = fHeightRaw / _maxHeight;
			float red = fHeight;
			float green = ((1.f - fHeight));
			Renderer::ParticleVertex vert{ glm::vec3(x,fHeightRaw,-y),glm::vec4(red,green,0.0,1.0f) };
			vertices.push_back(vert);
		}
	}
	_particles.reset(Renderer::ParticleSwarm::Create(_pDevice, vertices.data(), (int)vertices.size(), glm::vec2(0.7f)));
	return true;
}

void HEIGHTMAP::Render(glm::mat4&viewProj,glm::vec3&eye)
{
	vec2 size= _heightMapTexture->GetScale();
	mat4 sca = glm::scale(mat4(1.f), vec3(size.x, size.y, 1.f));
	_sprite->SetTransform(sca);

	_sprite->Draw(_heightMapTexture.get(), glm::vec3(0.f, 0.f, 0.f));
	_particles->Draw(viewProj, eye);
}
