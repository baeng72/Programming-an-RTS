#include "City.h"

CITY::CITY()
{
}

void CITY::Init(INTPOINT size)
{
	_size = size;
	_objects.clear();

	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			//Add tile
			_objects.push_back(OBJECT(TILE, vec3(x * TILE_SIZE, 0.f, y * -TILE_SIZE), vec3(0.f), vec3(1.f)));

			//Add house
			float sca_xz = rand() % 100 / 1000.f - 0.05f;
			float sca_y = rand() % 500 / 1000.f - 0.25f;
			int rotation = rand() % 4;
			int house = rand() % 2 + 1;
			if (x % 3 == 0 && y % 3 == 0)
				house = PARK;
			_objects.push_back(OBJECT(house, vec3(x * TILE_SIZE, 0.f, y * -TILE_SIZE), vec3(0.f, (glm::pi<float>() / 2.f) * rotation, 0.f), vec3(1.f + sca_xz, 1.f + sca_y, 1.f + sca_xz)));
		}
	}
}

void CITY::Render(CAMERA* cam,mat4&viewProj,Renderer::DirectionalLight&light)
{
	for (int i = 0; i < _objects.size(); i++) {
		if (cam == nullptr) {
			if (_objects[i]._rendered)
				_objects[i].Render(viewProj, light);
		}
		else if (cam->Cull(_objects[i]._BSphere)) {//Sphere culling
			_objects[i]._rendered = false;
		}
		else {
			_objects[i].Render(viewProj, light);
			_objects[i]._rendered = true;
		}
	}
}

vec3 CITY::GetCenter()
{
	return vec3(_size.x / 2.f * TILE_SIZE, 0.f, _size.y / 2.f * -TILE_SIZE);
}
