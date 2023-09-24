#include "mapObject.h"

std::unique_ptr<Renderer::Line2D> line;

void LoadMapObjectResources(Renderer::RenderDevice* pdevice) {
	line = std::unique_ptr<Renderer::Line2D>(Renderer::Line2D::Create(pdevice));
}

void UnloadMapObjectResources() {
	line.reset();
}

INTPOINT GetScreenPos(mat4&matVP,vec4&viewport, vec3& pos) {
	vec3 p = glm::project(pos, glm::mat4(1.f), matVP, viewport);
	return INTPOINT((int)p.x, (int)p.y);
}

// MAPOJBECT
MAPOBJECT::MAPOBJECT() {
	//Sets all variables to 0, nullptr or false
	_isBuilding = false;
	_hp = _hpMax = 0;
	_range = _damage = 0;
	_sightRadius = 0.f;
	_team = _type = 0;
	_selected = _dead = false;
	_pTarget = nullptr;
	_pDevice = nullptr;
}

Rect MAPOBJECT::GetMapRect(int border) {
	Rect mr = {
		_mappos.x - border,
		_mappos.y - border,
		_mappos.x + _mapsize.x + border,
		_mappos.y + _mapsize.y + border
	};
	return mr;
}

void MAPOBJECT::PaintSelected(mat4&matVP,vec4&viewport) {
	if (!_selected || _pDevice == nullptr)
		return;

	BBOX bbox = GetBoundingBox();	//Bounding box in world space

	//Create 8 points according to the cornders of the bounding box
	vec3 corners[] = { vec3(bbox.max.x, bbox.max.y, bbox.max.z),
							 vec3(bbox.max.x, bbox.max.y, bbox.min.z),
							 vec3(bbox.max.x, bbox.min.y, bbox.max.z),
							 vec3(bbox.max.x, bbox.min.y, bbox.min.z),
							 vec3(bbox.min.x, bbox.max.y, bbox.max.z),
							 vec3(bbox.min.x, bbox.max.y, bbox.min.z),
							 vec3(bbox.min.x, bbox.min.y, bbox.max.z),
							 vec3(bbox.min.x, bbox.min.y, bbox.min.z) };

	//Find the max and min points of these 8 offsets points in screen space
	INTPOINT pmax(-10000, -10000), pmin(10000, 10000);

	for (int i = 0; i < 8; i++) {
		INTPOINT screenPos = GetScreenPos(matVP, viewport, corners[i]);

		if (screenPos.x > pmax.x)
			pmax.x = screenPos.x;
		if (screenPos.y > pmax.y)
			pmax.y = screenPos.y;
		if (screenPos.x < pmin.x)
			pmin.x = screenPos.x;
		if (screenPos.y < pmin.y)
			pmin.y = screenPos.y;
	}

	Rect scr = { -20,-20, 820, 620 };

	//Check that the max and min point is within our viewport boundaries
	if (pmax.inRect(scr) || pmin.inRect(scr)) {
		float s = (pmax.x - pmin.x) / 3.f;

		vec2 corner1[] = { vec2(pmin.x, pmin.y + s), vec2(pmin.x, pmin.y), vec2(pmin.x + s, pmin.y) };
		vec2 corner2[] = { vec2(pmax.x - s, pmin.y), vec2(pmax.x, pmin.y), vec2(pmax.x, pmin.y + s) };
		vec2 corner3[] = { vec2(pmax.x, pmax.y - s), vec2(pmax.x, pmax.y), vec2(pmax.x - s, pmax.y) };
		vec2 corner4[] = { vec2(pmin.x + s, pmax.y), vec2(pmin.x, pmax.y), vec2(pmin.x, pmax.y - s) };

		//Draw the 4 corners
		line->Draw(corner1, 3, Color(1.f), 2.f);
		line->Draw(corner2, 3, Color(1.f), 2.f);
		line->Draw(corner3, 3, Color(1.f), 2.f);
		line->Draw(corner4, 3, Color(1.f), 2.f);

	}
}