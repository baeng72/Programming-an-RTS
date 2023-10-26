#pragma once
#include "heightMap.h"
#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>


HEIGHTMAP::HEIGHTMAP(INTPOINT size_,float maxHeight)
	:_size(size_),_maxHeight(maxHeight)
{	
	_pHeightMap = new float[_size.x * _size.y];
	memset(_pHeightMap, 0, sizeof(float) * _size.x * _size.y);

}

HEIGHTMAP::~HEIGHTMAP()
{
	delete[] _pHeightMap;
}

bool HEIGHTMAP::LoadFromFile(const char* fileName)
{
	
	glm::vec2 size(_size.x, _size.y);
	
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
			
			_pHeightMap[x + y * _size.x] = ((float)b / 255.f) * _maxHeight;
		}
	}
	return true;
}







void HEIGHTMAP::RaiseTerrain(Rect& r, float f)
{
	for (int y = r.top; y <= r.bottom; y++) {
		for (int x = r.left; x <= r.right; x++) {
			_pHeightMap[x + y * _size.x] += f;
			if (_pHeightMap[x + y * _size.x] < -_maxHeight)
				_pHeightMap[x + y * _size.x] = -_maxHeight;
			if (_pHeightMap[x + y * _size.x] > _maxHeight)
				_pHeightMap[x + y * _size.x] = _maxHeight;
		}
	}
	
}

void HEIGHTMAP::SmoothTerrain()
{
	float* hm = new float[_size.x * _size.y];
	memset(hm, 0, sizeof(float) * _size.x * _size.y);

	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			float totalHeight = 0.f;
			int noNodes = 0;
			//Add all neighboring heights together and use average
			for (int y1 = y - 1; y1 <= y + 1; y1++) {
				for (int x1 = x - 1; x1 <= x + 1; x1++) {
					if (x1 >= 0 && x1 < _size.x && y1 >= 0 && y1 < _size.y) {
						totalHeight += _pHeightMap[x1 + y1 * _size.x];
						noNodes++;
					}
				}
			}
			hm[x + y * _size.x] = totalHeight / (float)noNodes;
		}
	}
	delete[] _pHeightMap;
	_pHeightMap = hm;
	
}

void HEIGHTMAP::Cap(float capHeight)
{
	_maxHeight = 0.f;

	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			_pHeightMap[x + y * _size.x] -= capHeight;
			if (_pHeightMap[x + y * _size.x] < 0.f)
				_pHeightMap[x + y * _size.x] = 0.f;

			if (_pHeightMap[x + y * _size.x] > _maxHeight)
				_maxHeight = _pHeightMap[x + y * _size.x];
		}
	}
}

void HEIGHTMAP::operator*=(const HEIGHTMAP& rhs) {
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			float a = _pHeightMap[x + y * _size.x] / _maxHeight;
			float b = 1.f;
			if (x <= rhs._size.x && y <= rhs._size.y)
				b = rhs._pHeightMap[x + y * _size.x] / rhs._maxHeight;

			_pHeightMap[x + y * _size.x] = a * b * _maxHeight;
		}
	}
}