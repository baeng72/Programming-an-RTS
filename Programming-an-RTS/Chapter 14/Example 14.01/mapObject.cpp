#include "mapObject.h"
#include "unit.h"
#include "building.h"
#include "groupai.h"

std::unique_ptr<Renderer::Line2D> line;

void LoadMapObjectResources(Renderer::RenderDevice* pdevice) {
	line = std::unique_ptr<Renderer::Line2D>(Renderer::Line2D::Create(pdevice));
	int width, height;
	pdevice->GetDimensions(&width, &height);
	line->Update(width, height);
}

void UnloadMapObjectResources() {
	line.reset();
}

INTPOINT GetScreenPos(mat4& matVP, vec4& viewport, vec3& pos) {
	vec3 p = Core::project(pos, glm::mat4(1.f), matVP, viewport);// glm::project(pos, glm::mat4(1.f), matVP, viewport);
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
	_pGroup = nullptr;
}

Rect MAPOBJECT::GetMapRect(int border) {
	Rect mr = {
		_mappos.x - border,
		_mappos.y - border,
		_mappos.x + _mapsize.x + border,
		_mappos.y + _mapsize.y + border
	};
	//clip rectangle to terrain border
	if (_pTerrain) {
		mr.left = std::max(mr.left, 0);
		mr.right = std::min(mr.right, _pTerrain->_size.x - 1);
		mr.top = std::max(mr.top, 0);
		mr.bottom = std::min(mr.bottom, _pTerrain->_size.y - 1);
	}
	return mr;
}

INTPOINT MAPOBJECT::GetAttackPos(INTPOINT from) {
	if (_pTerrain == nullptr || !_pTerrain->Within(_mappos))
		return INTPOINT(-1,-1);

	Rect r = GetMapRect(1);
	Rect mr = GetMapRect(0);

	INTPOINT bestDest(-1, -1);

	float dist = 10000.f;

	int loop = 0;

	//Find the closest available attacking position
	for (int y = r.top; y <= r.bottom; y++) {
		for (int x = r.left; x <= r.right; x++) {
			INTPOINT p(x, y);

			if (!p.inRect(mr)) {
				MAPTILE* ptile = _pTerrain->GetTile(p);
				float d = from.Distance(p);

				if (ptile != nullptr && ptile->_pMapObject == nullptr && d < dist) {
					dist = d;
					bestDest = p;
				}
			}
			if (loop++ > 100) {
				LOG_INFO("Loop > 1000 {0}", __FUNCTION__);
				LOG_INFO("R Left: {0}, Right: {1}, Top {2}, Bottom {3}", r.left, r.right, r.top, r.bottom);
				LOG_INFO("Mappos: {0}, {1}", _mappos.x, _mappos.y);
				return INTPOINT(-1, -1);
			}
		}
	}
	return bestDest;
}

void MAPOBJECT::PaintSelected(mat4& matProj,mat4&matView, vec4& viewport) {
	mat4 matVP = matProj * matView;
	if (_isBuilding) {
		BUILDING* build = (BUILDING*)this;
		if (build->_training)
			build->_pTrainingEffect->Render(matProj,matView);
	}

	if (!_selected)
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

std::vector<MAPOBJECT*> MAPOBJECT::GetTargetsWithinRange(int range) {
	std::vector<MAPOBJECT*> enemies;

	if (!_pTerrain)
		return enemies;

	Rect r = GetMapRect(range);

	for (int y = r.top; y <= r.bottom; y++) {
		for (int x = r.left; x <= r.right; x++) {
			MAPTILE* ptile = _pTerrain->GetTile(x, y);

			if (ptile && ptile->_pMapObject) {
				if (ptile->_pMapObject->_team != _team && !ptile->_pMapObject->_dead) {
					if (_mappos.Distance(INTPOINT(x, y)) <= range || _isBuilding) {
						enemies.push_back(ptile->_pMapObject);
					}
				}
			}
		}
	}

	return enemies;
}

MAPOBJECT* MAPOBJECT::BestTargetToAttack(std::vector<MAPOBJECT*>& enemies) {
	if (enemies.empty())
		return nullptr;

	int lowestCost = 10000;
	MAPOBJECT* bestTarget = nullptr;

	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i] && !enemies[i]->_dead) {
			INTPOINT attackPos = enemies[i]->GetAttackPos(_mappos);

			if (_pTerrain->Within(attackPos) && _pTerrain->PositionAccessible(this, attackPos)) {
				int cost = (int)(enemies[i]->_hp + _mappos.Distance(enemies[i]->_mappos) * 30);

				if (cost < lowestCost) {
					bestTarget = enemies[i];
					lowestCost = cost;
				}
			}
		}
	}
	return bestTarget;
}