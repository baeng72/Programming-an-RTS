#pragma once

#include "intpoint.h"



struct PARTICLE {
	glm::vec3 position;
	glm::vec4 color;
};

struct HEIGHTMAP {	
	INTPOINT		_size;
	float			_maxHeight;		//the height of the highest peak
	float*			_pHeightMap;	//array with height values	
	HEIGHTMAP(INTPOINT size_,float maxHeight);
	~HEIGHTMAP();
	bool LoadFromFile(const char* fileName);
	bool CreateRandomHeightMap(int seed, float noiseSize, float persistence, int octaves);	
	
	glm::vec2 GetCenter() {
		return glm::vec2(_size.x / 2.f, _size.y / 2.f);
	}	

	void RaiseTerrain(Rect& r, float f);
	void SmoothTerrain();
	void Cap(float capHeight);

	void operator*=(const HEIGHTMAP& rhs);

	float GetHeight(int x, int y);
	float GetHeight(INTPOINT p);

};