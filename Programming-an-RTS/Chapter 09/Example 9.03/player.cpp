#include "player.h"
std::unique_ptr<Renderer::Shader> shader;

std::unique_ptr<Renderer::Line2D> selLine;
void LoadPlayerResources(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager>&shaderManager) {	
	shader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.03/shaders/building.glsl")));
	selLine = std::unique_ptr<Renderer::Line2D>(Renderer::Line2D::Create(pdevice));
	

}

void UnloadPlayerResources() {
	//shader.reset();
	selLine.reset();
}



// PLAYER

PLAYER::PLAYER(int teamNo, vec4 teamCol, INTPOINT startPos, TERRAIN* terrain,Renderer::RenderDevice*pdevice)
	:_pdevice(pdevice){
	_teamNo = teamNo;
	_teamColor = teamCol;
	_pTerrain = terrain;
	_areaSelect = false;

	if (!_pTerrain)
		return;

	//Add a few buildings
	for (int i = 0; i < 3; i++) {
		INTPOINT p = FindClosestBuildingLocation(i, startPos);
		if (_pTerrain->Within(p))
			AddMapObject(i, p, true);
	}

	//Also add a few units
	for (int i = 0; i < 6; i++) {
		INTPOINT mp;
		bool ok = false;
		do {
			mp.x = rand() % 20 - 10 + startPos.x;
			mp.y = rand() % 20 - 10 + startPos.y;

			MAPTILE* tile = _pTerrain->GetTile(mp);
			if (tile) {
				ok = tile->_walkable && tile->_pMapObject == nullptr;
			}
		} while (!ok);
		AddMapObject(i % 3, mp, false);
	}
	
}

PLAYER::~PLAYER() {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i])
			delete _mapObjects[i];
	}
}

void PLAYER::AddMapObject(int type, INTPOINT mp, bool isBuilding) {
	if (isBuilding)
		_mapObjects.push_back(new BUILDING(type, _teamNo, mp, _pTerrain,false));
	else
		_mapObjects.push_back(new UNIT(type, _teamNo, mp, _pTerrain));
}

void PLAYER::RenderMapObjects(CAMERA& camera, Renderer::DirectionalLight& light) {
	//EASY_FUNCTION(profiler::colors::Magenta);
	mat4 matView = camera.GetViewMatrix();
	mat4 matProj = camera.GetProjectionMatrix();
	mat4 matVP = matProj * matView;
	
	//Render units
	struct UBO {
		mat4 matVP;
		Renderer::DirectionalLight light;
		
	}ubo = { matVP,light };
	
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			//EASY_BLOCK("Cull and draw");
			if (!camera.Cull(_mapObjects[i]->GetBoundingBox())) {	
				Renderer::Shader* pshader = _mapObjects[i]->_isBuilding ? shader.get():_mapObjects[i]->GetShader();
				pshader->SetUniformData("UBO", &ubo, sizeof(ubo));
				_mapObjects[i]->Render(pshader);
			}
		}
	}
}

void PLAYER::PaintSelectedMapObjects(CAMERA& camera) {
	mat4 matView = camera.GetViewMatrix();
	mat4 matProj = camera.GetProjectionMatrix();
	mat4 matVP = matProj * matView;
	int width, height;
	_pdevice->GetDimensions(&width, &height);
	vec4 viewport{ 0,0,(float)width,(float)height };
	//Paint selected map objects
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			if (!camera.Cull(_mapObjects[i]->GetBoundingBox())) {
				_mapObjects[i]->PaintSelected(matVP, viewport);
			}
		}
	}
}

void PLAYER::UpdateMapObjects(float deltaTime) {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			_mapObjects[i]->Update(deltaTime);
		}
	}
}

INTPOINT PLAYER::FindClosestBuildingLocation(int buildType, INTPOINT mp) {
	//i = search radius
	for (int i = 0; i < 30; i++) {
		for (int x = mp.x - i; x <= mp.x + i; x++) {
			if (PlaceOk(buildType, INTPOINT(x, mp.y - i), _pTerrain))
				return INTPOINT(x, mp.y - i);
			if (PlaceOk(buildType, INTPOINT(x, mp.y + i), _pTerrain))
				return INTPOINT(x, mp.y + i);
		}
		for (int y = mp.y - i; y <= mp.y + i; y++) {
			if (PlaceOk(buildType, INTPOINT(mp.x - i, y), _pTerrain))
				return INTPOINT(mp.x - i, y);
			if (PlaceOk(buildType, INTPOINT(mp.x + i, y), _pTerrain))
				return INTPOINT(mp.x + i, y);
		}
	}
	//no good place found
	return INTPOINT(-1,-1);
}

void PLAYER::Select(mat4& matProj, mat4& matView, MOUSE& mouse) {
	mat4 matVP = matProj * matView;
	int width, height;
	_pdevice->GetDimensions(&width, &height);
	vec4 viewport{ 0,0,(float)width,(float)height };
	if (mouse.ClickLeft()) {
		//If the mouse button is pressed
		for (int i = 0; i < _mapObjects.size(); i++) {
			//Deselect all _mapObjects
			_mapObjects[i]->_selected = false;
		}

		if (!_areaSelect) {
			//If no area selction is in progress
			//Find closest _mapObjects
			int mapObject = -1;
			float bestDist = 100000.f;
			mat4 matWorld = mat4(1.f);
			RAY ray = mouse.GetRay(matProj, matView, matWorld);
			for (int i = 0; i < _mapObjects.size(); i++) {
				float dist = ray.Intersect(_mapObjects[i]->GetBoundingBox());

				if (dist >= 0.f && dist < bestDist) {
					mapObject = i;
					bestDist = dist;
				}
			}
			if (mapObject > -1)
				_mapObjects[mapObject]->_selected = true;
			else {
				_areaSelect = true;	//if no unit found
				_startSel = mouse;	//start area selection
			}
		}
		else {
			//Area selection in progress
			//Create area rectangle
			INTPOINT p1 = _startSel;
			INTPOINT p2 = mouse;
			if (p1.x > p2.x) {
				std::swap(p1.x, p2.x);
			}
			if (p1.y > p2.y) {
				std::swap(p1.y, p2.y);
			}
			Rect selRect = { p1.x,p1.y,p2.x,p2.y };

			//Draw selection rectangle
			vec2 box[] = { vec2(p1.x, p1.y), vec2(p2.x, p1.y),
								 vec2(p2.x, p2.y), vec2(p1.x, p2.y),
								 vec2(p1.x, p1.y) };
			selLine->Draw(box, 5, Color(1.f), 1.f);

			//select any units inside our rectangle
			for (int i = 0; i < _mapObjects.size(); i++) {
				if (_mapObjects[i] && !_mapObjects[i]->_isBuilding) {
					INTPOINT p = GetScreenPos(matVP, viewport, _mapObjects[i]->_position);
					if (p.inRect(selRect))
						_mapObjects[i]->_selected = true;
				}
			}
		}
	}
	else {
		_areaSelect = false;
	}
}

void PLAYER::UnitOrders(MOUSE& mouse) {
	if (mouse.ClickRight()) {
		mouse.DisableInput(300);

		for (int i = 0; i < _mapObjects.size(); i++) {
			if (_mapObjects[i]) {
				if (!_mapObjects[i]->_isBuilding && _mapObjects[i]->_selected) {
					//cast to unit
					UNIT* unit = dynamic_cast<UNIT*>(_mapObjects[i]);
					unit->Goto(mouse._mappos, false, true);
				}
			}
		}
	}
}

INTPOINT PLAYER::GetCenter() {
	INTPOINT p;
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			p += _mapObjects[i]->_mappos;
		}
	}
	p /= (int) _mapObjects.size();
	return p;
}

