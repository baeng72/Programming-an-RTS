#include "player.h"
std::unique_ptr<Renderer::Shader> shader;
std::unique_ptr<Renderer::Line2D> selLine;
std::unique_ptr<Renderer::Sprite> menuSprite;
std::unique_ptr<Renderer::Texture> uiTexture;
std::unique_ptr<Renderer::Font> fontMoney;

void LoadPlayerResources(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager) {
	
	shader.reset(Renderer::Shader::Create(pdevice, shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("building.glsl"))));
	
	selLine = std::unique_ptr<Renderer::Line2D>(Renderer::Line2D::Create(pdevice));

	menuSprite.reset(Renderer::Sprite::Create(pdevice));
	
	
	uiTexture.reset(Renderer::Texture::Create(pdevice,Core::ResourcePath::GetTexturePath("ui.png"),vec2(-1)));

	fontMoney.reset(Renderer::Font::Create());
	fontMoney->Init(pdevice, Core::ResourcePath::GetFontPath("arialn.ttf"), 18);


}

void UnloadPlayerResources() {
	//shader.reset();'
	fontMoney.reset();
	selLine.reset();

	menuSprite.reset();

	uiTexture.reset();
}

float GetCost(int type, bool isBuilding) {
	if (isBuilding) {
		switch (type) {
		case TOWNHALL:
			return 25.f;
		case BARRACKS:
			return 20.f;
		case TOWER:
			return 30.f;
		}
	}
	else
	{
		switch (type) {
		case WORKER:
			return 7.f;
		case SOLDIER:
			return 12.f;
		case MAGICIAN:
			return 15.f;
		}
	}
	return 100.f;
}


// PLAYER

PLAYER::PLAYER(int teamNo, int controller,vec4 teamCol, INTPOINT startPos, TERRAIN* terrain, Renderer::RenderDevice* pdevice)
	:_pdevice(pdevice) {
	_unitUpdateIndex = -1;
	_teamNo = teamNo;
	_teamColor = teamCol;
	_pTerrain = terrain;
	_areaSelect = false;
	_placeBuilding = false;
	_nextUnitUpdate = 0.5f;
	_unitUpdateIndex = 0;
	_time = 0.f;
	_controller = controller;
	_pAi = nullptr;
	money = 0.f;
	unitLimit = 15;
	_teamStartLocation = startPos;
	_numKills = 0;
	if (!_pTerrain)
		return;

	//Add a few buildings
	for (int i = 0; i < 0; i++) {
		INTPOINT p = FindClosestBuildingLocation(i, startPos);
		if (_pTerrain->Within(p))
			AddMapObject(i, p, true, true);
	}

	//Also add a few units
	for (int i = 0; i < 1; i++) {
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
		AddMapObject(i % 3, mp, false, true);
	}

	if (_controller == COMPUTER) {
		_pAi = new MASTERAI(this,_pTerrain);
	}

}

PLAYER::~PLAYER() {
	if (_pAi)
		delete _pAi;
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i])
			delete _mapObjects[i];
	}
}

MAPOBJECT* PLAYER::AddMapObject(int type, INTPOINT mp, bool isBuilding,bool finished) {
	if (isBuilding)
		_mapObjects.push_back(new BUILDING(_pdevice,type, _teamNo,finished, mp, _pTerrain,this, true));
	else
		_mapObjects.push_back(new UNIT(_pdevice,type, _teamNo, mp, _pTerrain,this));
	return _mapObjects[_mapObjects.size() - 1];
}

void PLAYER::RemoveMapObject(MAPOBJECT* pobj) {
	auto & it=std::find(_mapObjects.begin(), _mapObjects.end(), pobj);
	if (it != _mapObjects.end()) {
		_mapObjects.erase(it);
	}
}

void PLAYER::RenderMapObjects(CAMERA& camera, Renderer::DirectionalLight& light) {
	//EASY_FUNCTION(profiler::colors::Magenta);
	mat4 matView = camera.GetViewMatrix();
	mat4 matProj = camera.GetProjectionMatrix();
	mat4 matVP = matProj * matView;

	//Render units
	

	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			//EASY_BLOCK("Cull and draw");
			if (!camera.Cull(_mapObjects[i]->GetBoundingBox())) {
				Renderer::Shader* pshader;
				if (_mapObjects[i]->_isBuilding) {
					pshader = shader.get();
					//pshader->Bind();
				}
				else {
					pshader = _mapObjects[i]->GetShader();					
				}
				
				pshader->SetUniform("viewProj", &matVP);
				pshader->SetUniform("light.ambient", &light.ambient);
				pshader->SetUniform("light.diffuse", &light.diffuse);
				pshader->SetUniform("light.specular", &light.specular);
				pshader->SetUniform("light.direction", &light.direction);
				
				_mapObjects[i]->Render(pshader);
			}
		}
	}
	//render building fires
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i] && _mapObjects[i]->_isBuilding) {
			BUILDING* pbuild = (BUILDING*)_mapObjects[i];
			pbuild->RenderFires(matProj, matView);
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
				_mapObjects[i]->PaintSelected(matProj,matView, viewport);
			}
		}
	}
}

void PLAYER::UpdateMapObjects(float deltaTime) {
	//player time
	_time += deltaTime * 0.05f;
	if (_time > 1.f)
		_time -= 1.f;
	money += deltaTime;
	if (money > 100.f)
		money = 100.f;

	//update map objects
	std::vector<MAPOBJECT*> toDelete;
	for (auto pobj : _mapObjects) {
		UNIT* punit = (UNIT*)pobj;
		if (punit) {
			if (punit->isDead())
				toDelete.push_back(punit);
			else
				punit->Update(deltaTime);

		}
	}
	for (auto p : toDelete) {
		_mapObjects.erase(std::find(_mapObjects.begin(), _mapObjects.end(), p));
	}
	
	_nextUnitUpdate -= deltaTime;
	if (_nextUnitUpdate < 0.f && !_mapObjects.empty()) {
		_unitUpdateIndex++;
		if(_unitUpdateIndex >= _mapObjects.size())
			_unitUpdateIndex = 0;

		_nextUnitUpdate = 0.5f / (float)_mapObjects.size();

		if (!_mapObjects[_unitUpdateIndex]->_isBuilding) {
			UNIT* punit = (UNIT*)_mapObjects[_unitUpdateIndex];
			punit->UnitAI(false);
		}
	}
	//Master AI
	if (_controller == COMPUTER && _pAi) {
		_pAi->Update(deltaTime);
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
	return INTPOINT(-1, -1);
}

void PLAYER::Select(mat4& matProj, mat4& matView, MOUSE& mouse) {
	mat4 matVP = matProj * matView;
	int width, height;
	_pdevice->GetDimensions(&width, &height);
	vec4 viewport{ 0,0,(float)width,(float)height };
	if (mouse.ClickLeft()) {//If the mouse button is pressed
		//if user preseed in the menu or minimap don't start the selection process (this is hardcoded, needs to read values if minimap moves)
		Rect minimap{ 611,9,791,189 };
		Rect UI = { 650, 430, 800, 600 };
		if (mouse.inRect(minimap))
			return;
		if (mouse.inRect(UI))
			return;

		
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

void PLAYER::UnitOrders(MOUSE& mouse,std::vector<PLAYER*>&players, CAMERA&camera) {
	if (_placeBuilding)return;
	//if user preseed in the menu or minimap don't start the selection process (this is hardcoded, needs to read values if minimap moves)
	Rect minimap{ 611,9,791,189 };
	Rect UI = { 650, 430, 800, 600 };

	if (mouse.ClickRight()) {
		if (mouse.inRect(minimap))
			return;
		if (mouse.inRect(UI))
			return;
	
		mouse.DisableInput(300);

		mat4 matProj = camera.GetProjectionMatrix();
		mat4 matView = camera.GetViewMatrix();
		mat4 matVP = matProj * matView;

		RAY ray = mouse.GetRay(matProj, matView, mat4(1.f));

		MAPOBJECT* penemytarget = nullptr;
		float bestDist = 100000.f;

		//see what enemy target the ray intersects
		for (int p = 0; p < players.size(); p++) {
			if (players[p] && players[p]->_teamNo != _teamNo) {
				for (int m = 0; players[p]->_mapObjects.size(); m++) {
					if (players[p]->_mapObjects[m]) {
						float dist = ray.Intersect(players[p]->_mapObjects[m]->GetBoundingBox());

						if (dist > 0.f && dist < bestDist) {
							penemytarget = players[p]->_mapObjects[m];
							bestDist = dist;
						}
					}
				}
			}
		}
		//give orders

		for (int i = 0; i < _mapObjects.size(); i++) {
			if (_mapObjects[i]) {
				if (!_mapObjects[i]->_isBuilding && _mapObjects[i]->_selected) {
					//cast to unit
					UNIT* unit = dynamic_cast<UNIT*>(_mapObjects[i]);
					

					if (penemytarget) {
						unit->Attack(penemytarget);
					}
					else {
						unit->Goto(mouse._mappos, false, true,STATE_MOVING);
					}
				}
			}
		}
	}
}

BUILDING* PLAYER::GetAvailableBuilding(int type) {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i] && _mapObjects[i]->_type == type &&
			_mapObjects[i]->_isBuilding && !_mapObjects[i]->_dead) {
			BUILDING* pbuilding = (BUILDING*)_mapObjects[i];
			if (pbuilding->_buildProgress >= 1.f && !pbuilding->_training)
				return pbuilding;
		}
	}
	return nullptr;
}

UNIT* PLAYER::GetAvailableUnit(int type) {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i] && _mapObjects[i]->_type == type &&
			!_mapObjects[i]->_isBuilding && !_mapObjects[i]->_dead) {
			UNIT* punit = (UNIT*)_mapObjects[i];
			if (punit->_state == STATE_IDLE)
				return punit;
		}
	}
	return nullptr;
}

INTPOINT PLAYER::GetCenter() {
	INTPOINT p;
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			p += _mapObjects[i]->_mappos;
		}
	}
	p /= (int)_mapObjects.size();
	return p;
}

Rect PLAYER::GetBaseArea() {
	Rect r = { 10000,10000,-10000,-10000 };

	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i] && _mapObjects[i]->_isBuilding) {
			Rect mr = _mapObjects[i]->GetMapRect(5);

			r.left = std::min(r.left, mr.left);
			r.right = std::min(r.right, mr.right);
			r.top = std::min(r.top, mr.top);
			r.bottom = std::min(r.bottom, mr.bottom);
		}
	}
	return r;
}


void PLAYER::IsMapObjectsVisible() {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]) {
			if (_mapObjects[i]->_isBuilding) {
				Rect r = _mapObjects[i]->GetMapRect(0);
				_mapObjects[i]->_visible = false;

				for (int y = r.top; y <= r.bottom && !_mapObjects[i]->_visible; y++) {
					for (int x = r.left; x <= r.right && !_mapObjects[i]->_visible; x++) {
						if (_pTerrain->_visitedTiles[x + y * _pTerrain->_size.x]) {
							_mapObjects[i]->_visible = true;
						}
					}
				}
			}
			else {
				INTPOINT mp = _mapObjects[i]->_mappos;
				_mapObjects[i]->_visible = _pTerrain->_visibleTiles[mp.x + mp.y * _pTerrain->_size.x];
			}
		}
	}
}

bool PLAYER::HasMapObject(int type, bool isBuilding) {
	for (int i = 0; i < _mapObjects.size(); i++) {
		if (_mapObjects[i]->_isBuilding == isBuilding) {
			if (_mapObjects[i]->_type == type)
				return true;
		}
	}
	return false;
}