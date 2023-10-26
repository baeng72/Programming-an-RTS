#pragma once
#include "heightMap.h"
#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>


HEIGHTMAP::HEIGHTMAP(Renderer::RenderDevice* pdevice, INTPOINT size_)
	:_size(size_),_pDevice(pdevice),_maxHeight(15.f)
{
	_sprite.reset(Renderer::Sprite::Create(_pDevice));
	_pHeightMap = new float[_size.x * _size.y];
}

HEIGHTMAP::~HEIGHTMAP()
{
	delete[] _pHeightMap;
}

bool HEIGHTMAP::LoadFromFile(const char* fileName)
{
	
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
	//_pHeightMap = new float[_size.x * _size.y];
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

float Noise(int x) {
	x = (x << 13) ^ x;
	return (1.f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.f);
}

float CosInterpolate(float v1, float v2, float a) {
	float angle = a * glm::pi<float>();
	float prc = (1.f - cos(angle)) * 0.5f;
	return v1 * (1.f - prc) + v2 * prc;
}
bool HEIGHTMAP::CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves) {
	uint8_t* pixels = new uint8_t[_size.x * _size.y];
	//For each map node
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			//Scale x & y to the range of 0.0 - _size
			float xf = ((float)x / (float)_size.x) * noiseSize;
			float yf = ((float)y / (float)_size.y) * noiseSize;

			float total = 0;

			//For each octave
			for (int i = 0; i < octaves; i++) {
				//Calculate frequency and amplitude (different for each octave)
				float freq = std::powf(2, (float)i);
				float amp = std::powf(persistence, (float)i);

				//Calculate the x, y noise coordinates
				float tx = xf * freq;
				float ty = yf * freq;
				int tx_int = (int)tx;
				int ty_int = (int)ty;

				//Calculate fractions of x & y
				float fracX = tx - tx_int;
				float fracY = ty - ty_int;

				//Get the noise of this octave for each of these 4 points
				float v1 = Noise(tx_int + ty_int * 57 + seed);
				float v2 = Noise(tx_int + 1 + ty_int * 57 + seed);
				float v3 = Noise(tx_int + (ty_int + 1) * 57 + seed);
				float v4 = Noise(tx_int + 1 + (ty_int + 1) * 57 + seed);

				//Smooth in the X-axis
				float i1 = CosInterpolate(v1, v2, fracX);
				float i2 = CosInterpolate(v3, v4, fracX);

				//Smooth in the Y-axis
				total += CosInterpolate(i1, i2, fracY) * amp;
			}

			int b = (int)(128 + total * 128.f);
			if (b < 0)
				b = 0;
			if (b > 255)
				b = 255;
			pixels[y * _size.x + x] = b;
			_pHeightMap[x + y * _size.x] = ((float)b / 255.f) * _maxHeight;
		}
	}
	_heightMapTexture.reset(Renderer::Texture::Create(_pDevice, _size.x, _size.y, 1, pixels));
	delete[] pixels;
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
	_sprite->Draw(_heightMapTexture.get(), glm::vec3(1.f, 1.f, 0.f));
	_particles->Draw(viewProj, eye);
}
