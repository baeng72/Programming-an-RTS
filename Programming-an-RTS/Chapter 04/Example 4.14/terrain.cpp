#include "terrain.h"


PATCH::PATCH()
{
	_pdevice = nullptr;
	
}

PATCH::~PATCH()
{
	Release();
}

bool PATCH::CreateMesh(TERRAIN&t, Rect source, Renderer::RenderDevice* pdevice)
{
	_pdevice = pdevice;
	int width = source.right - source.left;
	int height = source.bottom - source.top;
	int nrVert = (width + 1) * (height + 1);
	int nrTri = width * height * 2;
	float invSizeX = 1 / (float)t._size.x;
	float invSizeY = 1 / (float)t._size.y;
	std::vector<TERRAINVertex> vertices(nrVert);
	for (int z = source.top, z0 = 0; z <= source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x <= source.right; x++, x0++) {
			//Strect UV coordinates once over the entire terrain
			
			glm::vec2 alphaUV = glm::vec2(x * invSizeX, z * invSizeY);
			glm::vec2 uv = alphaUV * 8.f;
			//Extract height (and position) from heightMap
			MAPTILE* ptile = t.GetTile(x, z);
			glm::vec3 pos = glm::vec3(x, ptile->_height, -z);

			vertices[z0 * (width + 1) + x0] = TERRAINVertex(pos,uv,alphaUV);
		}
	}
	std::vector<uint32_t> indices(3 * nrTri);
	uint32_t indexCount = 0;
	for (int z = source.top, z0 = 0; z < source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x < source.right; x++, x0++) {
			//Triangle 1
			indices[indexCount++] = z0 * (width + 1) + x0;
			indices[indexCount++] = z0 * (width + 1) + x0 + 1;
			indices[indexCount++] = (z0 + 1) * (width + 1) + x0;
			//Triangle 2
			indices[indexCount++] = (z0 + 1) * (width + 1) + x0;
			indices[indexCount++] = z0 * (width + 1) + x0 + 1;
			indices[indexCount++] = (z0 + 1) * (width + 1) + x0 + 1;
		}
	}
	
	//compute normals. sum vertex normals
	std::vector<glm::vec3> normals(nrTri);//triangle normals
	for (int32_t i = 0, idx = 0; i < nrTri; i++, idx += 3) {
		uint32_t i0 = indices[idx + 0];
		uint32_t i1 = indices[idx + 1];
		uint32_t i2 = indices[idx + 2];

		glm::vec3 n0 = vertices[i0].position;
		glm::vec3 n1 = vertices[i1].position;
		glm::vec3 n2 = vertices[i2].position;

		//calc triangle normal
		glm::vec3 edge0 = n1 - n0;
		glm::vec3 edge1 = n2 - n0;

		//cross
		glm::vec3 norm = glm::cross(edge0, edge1);//order might be wrong? 
		normals[i] = norm;// glm::normalize(norm);//need to normalize?
	}

	//dumb way to sum normals for a vertex?
	for (int32_t i = 0; i < nrVert; i++) {
		glm::vec3 n = glm::vec3(0.f);
		int32_t numNorm = 0;
		for (int j = 0, idx = 0; j < nrTri; j++, idx += 3) {
			uint32_t i0 = indices[idx + 0];
			uint32_t i1 = indices[idx + 1];
			uint32_t i2 = indices[idx + 2];
			if (i0 == i || i1 == i || i2 == i) {
				n += normals[j];
				numNorm++;
			}
		}
		if (numNorm > 0) {
			n /= numNorm;
		}
		else {
			n = glm::vec3(0.f, 1.f, 0.f);
		}
		vertices[i].normal = glm::normalize(n);
	}
	Renderer::VertexAttributes attributes = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2,Renderer::ShaderDataType::Float2},sizeof(TERRAINVertex) };
		_mesh.reset(Mesh::Mesh::Create(pdevice, (float*)vertices.data(), sizeof(TERRAINVertex) * nrVert, indices.data(), indexCount * sizeof(uint32_t),attributes));
	
	
	return false;
}

void PATCH::Release() {
	_mesh.reset();
}

void PATCH::Render()
{
	_mesh->Bind();
	_mesh->Render();
}


TERRAIN::TERRAIN()
{
	_pMaptiles = nullptr;
	_pdevice = nullptr;
	
}
void TERRAIN::Cleanup()
{
	Release();
	if (_pMaptiles)
		delete[] _pMaptiles;
}
void TERRAIN::Release() {
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	for (int i = 0; i < _patches.size(); i++) {
		if (_patches[i])
			delete _patches[i];
	}
	_patches.clear();

	if (_heightMap) {
		_heightMap.reset();
	}

	_objects.clear();
}

void TERRAIN::SetWireframe(bool wireframe)
{
	_shader->SetWireframe(wireframe);	
}

void TERRAIN::Init(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager, INTPOINT size_)
{
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	_size = size_;

	if (_pMaptiles != nullptr)//clear old maptiiles
		delete[] _pMaptiles;
	//Create maptiles
	_pMaptiles = new MAPTILE[_size.x * _size.y];
	memset(_pMaptiles, 0, sizeof(MAPTILE) * _size.x * _size.y);

	//Load Textures
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("grass.jpg"))));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("mountain.jpg"))));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("snow.jpg"))));
	
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("terrain.glsl"), Renderer::ShaderCullMode::frontFace)));
	

	GenerateRandomTerrain(3);
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	_pdevice->Wait();
	Release();

	//Create two heightmaps and multiply them

	_heightMap = std::make_unique<HEIGHTMAP>(_size, 20.f);
	HEIGHTMAP hm2(_size, 30.f);

	_heightMap->CreateRandomHeightMap(rand() % 2000, 2.5f, 0.5f, 8);
	hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.8f, 3);
	hm2.Cap(hm2._maxHeight * 0.4f);

	*_heightMap *= hm2;

	
	//Add Objects
	HEIGHTMAP hm3(_size, 1.f);
	hm3.CreateRandomHeightMap(rand() % 1000, 5.5f, 0.9f, 7);

	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			if (_heightMap->GetHeight(x, y) == 0.f && hm3.GetHeight(x, y) > 0.7f && rand() % 6 == 0)
				AddObject(0, INTPOINT(x, y));	//tree
			else if (_heightMap->GetHeight(x, y) >= 1.f && hm3.GetHeight(x, y) > 0.9f && rand() % 20 == 0)
				AddObject(1, INTPOINT(x, y));//stone
		}
	}
	
	InitPathfinding();
		
	CreatePatches(numPatches);
		
	CalculateAlphaMaps();
		
}

void TERRAIN::CreatePatches(int numPatches)
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	
	for (int i = 0; i < _patches.size(); i++) {
		if (_patches[i])
			delete _patches[i];
	}
	_patches.clear();
	//Create new patches
	for (int y = 0; y < numPatches; y++) {
		for (int x = 0; x < numPatches; x++) {
			Rect r = { (int)(x * (_size.x - 1) / (float)numPatches),
			(int)(y * (_size.y - 1) / (float)numPatches),
			(int)((x + 1) * (_size.x - 1) / (float)numPatches),
			(int)((y + 1) * (_size.y - 1) / (float)numPatches) };

			PATCH* p = new PATCH();
			p->CreateMesh(*this, r, _pdevice);
			_patches.push_back(p);
		}
	}
	
}

void TERRAIN::CalculateAlphaMaps() {
	//height ranges
	
	constexpr int texWidth = 128;
	constexpr int texHeight = 128;
	//create one alpha map per diffuse map
	uint32_t* pdata = new uint32_t[texWidth*texHeight];
	memset(pdata, 0, sizeof(uint32_t) * texWidth * texHeight);
	for (size_t i = 0; i < _diffuseMaps.size(); i++) {
		//for each pixel in the alphaMap
		uint32_t shift = (uint32_t)((2-i) << 3);
		for (int y = 0; y < texHeight; y++) {
			for (int x = 0; x < texWidth; x++) {
				int terrain_x = (int)(_size.x * (x / (float)(texWidth)));
				int terrain_y = (int)(_size.y * (y / (float)(texHeight)));
				MAPTILE* ptile = GetTile(terrain_x, terrain_y);
				float height = 0;
				uint32_t b = 0;
				if (ptile && ptile->_type==i)					
					b = 0xFF;
				else
					b = 0;
				b <<= shift;
				pdata[x + y * texWidth] |= b;
			}
		}
		
	}
	//create a new texture
	_alphaMap.reset(Renderer::Texture::Create(_pdevice, texWidth, texHeight, Renderer::TextureFormat::R8G8B8A8, (uint8_t*)pdata));
	
	delete[] pdata;
}

void TERRAIN::Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light)
{
	
	
	_shader->SetUniform("viewProj", &viewProj);
	_shader->SetUniform("model", &model);
	_shader->SetUniform("light.ambient", &light.ambient);
	_shader->SetUniform("light.diffuse", &light.diffuse);
	_shader->SetUniform("light.specular", &light.specular);
	_shader->SetUniform("light.direction", &light.direction);
	std::vector<Renderer::Texture*> textures = { _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get(),_alphaMap.get() };
	_shader->SetTextures(textures.data(), 4);
	_shader->Bind();
	for (size_t i = 0; i < _patches.size(); i++)
		_patches[i]->Render();
	

	//render object
	for (int i = 0; i < _objects.size(); i++) {
		_objects[i].Render(viewProj, light);
	}
}


void TERRAIN::AddObject(int type, INTPOINT mappos) {
	glm::vec3 pos = glm::vec3(mappos.x, _heightMap->GetHeight(mappos), -mappos.y);
	glm::vec3 rot = glm::vec3((rand() % 1000 / 1000.f) * 0.13f, (rand() / 1000 / 1000.f) * 3.f, (rand() % 1000 / 1000.f) * 0.13f);

	float sca_xz = (rand() % 1000 / 1000.f) * 0.5f + 0.5f;
	float sca_y = (rand() % 1000 / 1000.f) * 1.f + 0.5f;
	glm::vec3 sca = glm::vec3(sca_xz, sca_y, sca_xz);

	_objects.push_back(OBJECT(type,mappos, pos, rot, sca));
}

bool TERRAIN::Within(INTPOINT p)
{
	return p.x >= 0 && p.y >= 0 && p.x < _size.x&& p.y < _size.y;
}

void TERRAIN::InitPathfinding()
{
	//Read maptile heights & types from heightmap
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {

			MAPTILE* ptile = GetTile(x, y);
			if(_heightMap)
				ptile->_height = _heightMap->GetHeight(x, y);
			ptile->_mappos = INTPOINT(x, y);

			if (ptile->_height < 1.f)
				ptile->_type = 0;		//Grass
			else if (ptile->_height < 15.f)
				ptile->_type = 1;		//Stone
			else
				ptile->_type = 2;		//Snow
		}
	}

	//Calculate the tile cost as a function of the height variance
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {

			MAPTILE* ptile = GetTile(x, y);
			if (ptile) {
				//Possible neighbors
				INTPOINT p[] = { INTPOINT(x - 1,y - 1),	INTPOINT(x,y - 1),	INTPOINT(x + 1,y - 1),
								INTPOINT(x - 1,y),							INTPOINT(x + 1,y),
								INTPOINT(x - 1,y + 1),	INTPOINT(x,y + 1),	INTPOINT(x + 1,y + 1),
				};

				float variance = 0.f;
				int nr = 0;

				//For each neighbor
				for (int i = 0; i < numNeighbors; i++) {
					if (Within(p[i])) {
						MAPTILE* pneighbor = GetTile(p[i]);
						if (pneighbor) {
							float v = pneighbor->_height - ptile->_height;
							variance += (v * v);	//variance = sd^2 = (m-n)*(m-n)/nr or something
							nr++;
						}
					}
				}

				//cost = height variance
				variance /= (float)nr;
				ptile->_cost = variance + 0.1f;
				if (ptile->_cost > 1.f)
					ptile->_cost = 1.f;

				//if the tile cost is less than 1.f, then we can walk on the tile
				ptile->_walkable = ptile->_cost < 1.f;
			}
		}
	}
	//Make maptiles with objects on them not walkable
	for (int i = 0; i < _objects.size(); i++) {
		MAPTILE* ptile = GetTile(_objects[i]._mappos);
		if (ptile) {
			ptile->_walkable = false;
			ptile->_cost = 1.f;
		}
	}

	//Connect maptiles using the neighbors[] pointers
	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			MAPTILE* ptile = GetTile(x, y);
			if (ptile && ptile->_walkable) {
				//clear old connections
				for (int i = 0; i < numNeighbors; i++)
					ptile->_neighbors[i] = nullptr;

				//Possible neighbors
				INTPOINT p[] = { INTPOINT(x - 1,y - 1),	INTPOINT(x,y - 1),	INTPOINT(x + 1,y - 1),
								INTPOINT(x - 1,y),							INTPOINT(x + 1,y),
								INTPOINT(x - 1,y + 1),	INTPOINT(x,y + 1),	INTPOINT(x + 1,y + 1),
				};

				//For each neighbor
				for (int i = 0; i < numNeighbors; i++) {
					if (Within(p[i])) {
						MAPTILE* pneighbor = GetTile(p[i]);
						
						//connect tiles if the neighbor is walkable
						if (pneighbor && pneighbor->_walkable)
							ptile->_neighbors[i] = pneighbor;

					}
				}

			}
		}
	}
	CreateTileSets();
}

void TERRAIN::CreateTileSets()
{
	int setNo = 0;
	for (int y = 0; y < _size.y; y++) {
		// set a unique set for each tile
		for (int x = 0; x < _size.x; x++) {
			_pMaptiles[x + y * _size.x]._set = setNo++;
		}
	}

	bool changed = true;
	while (changed) {
		changed = false;

		for (int y = 0; y < _size.y; y++) {
			for (int x = 0; x < _size.x; x++) {
				MAPTILE* ptile = GetTile(x, y);

				//Find the lowest set of a neighbor
				if (ptile && ptile->_walkable) {
					for (int i = 0; i < numNeighbors; i++) {
						if (ptile->_neighbors[i] &&
							ptile->_neighbors[i]->_walkable &&
							ptile->_neighbors[i]->_set < ptile->_set) {
							changed = true;
							ptile->_set = ptile->_neighbors[i]->_set;
						}
					}
				}
			}
		}
	}
}

float H(INTPOINT a, INTPOINT b){
	//return abs(a.x - b.x) + abs(a.y-b.y);
	return a.Distance(b);
}

std::vector<INTPOINT> TERRAIN::GetPath(INTPOINT start, INTPOINT goal)
{
	//Check that the two points are within the bounds of the map
	MAPTILE* pstartTile = GetTile(start);
	MAPTILE* pgoalTile = GetTile(goal);
	if(!Within(start) ||!Within(goal) || start == goal || pstartTile==nullptr || pgoalTile==nullptr)
		return std::vector<INTPOINT>();

	//Check if a path exists
	if(!pstartTile->_walkable || !pgoalTile->_walkable || pstartTile->_set != pgoalTile->_set)
		return std::vector<INTPOINT>();

	//Init search
	long numTiles = _size.x * _size.y;
	for (long l = 0; l < numTiles; l++) {
		_pMaptiles[l].f = _pMaptiles[l].g = (float)INT_MAX;		//clear f, g
		_pMaptiles[l].open = _pMaptiles[l].closed = false;		//reset open and closed
	}

	std::vector<MAPTILE*> open;					//create our open list
	pstartTile->g = 0.f;						//init our starting point (SP)
	pstartTile->f = H(start, goal);
	pstartTile->open = true;
	open.push_back(pstartTile);					//Add SP to the open list

	bool found = false;						//search as long as a path hasn't been found
	while (!found && !open.empty()) {		//or there are no more tiles to search

		MAPTILE* pbest = open[0];			//find the best tile (i.e. the lowest F value)
		int bestPlace = 0;
		for (int i = 1; i < open.size(); i++) {
			if (open[i]->f < pbest->f) {
				pbest = open[i];
				bestPlace = i;
			}
		}

		if(pbest == nullptr)	
			break;							//no path found

		open[bestPlace]->open = false;
		//open.erase(&open[bestPlace]);		//Take the best node out of the Open list
		std::vector<MAPTILE*>::iterator iter = open.begin() + bestPlace;
		open.erase(iter);

		if (pbest->_mappos == goal) {			//if the goal has been found
			std::vector<INTPOINT> p, p2;
			MAPTILE* ppoint = pbest;
			while (ppoint->_mappos != start) {	//Generate path
				p.push_back(ppoint->_mappos);
				ppoint = ppoint->_pParent;
			}

			for (size_t i = p.size() - 1; i != 0; i-- )		//Reverse path
				p2.push_back(p[i]);
			p2.push_back(goal);
			return p2;
		}
		else {
			for (int i = 0; i < numNeighbors; i++) {			//otherwise, check the neighbors of the
				if (pbest->_neighbors[i]) {						//best tile
					bool inList = false;						//generate new G and F values
					float newG = pbest->g + 1.f;
					float d = H(pbest->_mappos, pbest->_neighbors[i]->_mappos);
					float newF = newG + H(pbest->_neighbors[i]->_mappos, goal) + pbest->_neighbors[i]->_cost * 5.f * d;
					
					if (pbest->_neighbors[i]->open || pbest->_neighbors[i]->closed) {
						if (newF < pbest->_neighbors[i]->f) {	//If the new F value is lower
							pbest->_neighbors[i]->g = newG;		//update the values of this tile
							pbest->_neighbors[i]->f = newF;
							pbest->_neighbors[i]->_pParent = pbest;
						}
						inList = true;
					}

					if (!inList) {								//if the neighbor tile isn't in the open or closed list
						pbest->_neighbors[i]->f = newF;			//set the values
						pbest->_neighbors[i]->g = newG;
						pbest->_neighbors[i]->_pParent = pbest;
						pbest->_neighbors[i]->open = true;
						open.push_back(pbest->_neighbors[i]);	//add it to the open list
					}
				}
			}
			pbest->closed = true;							//the best tile has now beed searched, add it to the closed list

		}
	}
	return std::vector<INTPOINT>();		//no path found
	
}

MAPTILE* TERRAIN::GetTile(int x, int y)
{
	return &_pMaptiles[x + y * _size.x];
}

void TERRAIN::SaveTerrain(const char* pfilename)
{
	std::ofstream out(pfilename, std::ios::binary);				//binary format
	if (out.good()) {
		out.write((char*)&_size, sizeof(INTPOINT));				//write map size

		//write all the maptile information needed to recreate the map
		for (int y = 0; y < _size.y; y++) {
			for (int x = 0; x < _size.x; x++) {
				MAPTILE* ptile = GetTile(x, y);
				out.write((char*)&ptile->_type, sizeof(int));			//type
				out.write((char*)&ptile->_height, sizeof(float));		//height
			}
		}

		//Write all the objects
		int numObjects = (int) _objects.size();
		out.write((char*)&numObjects, sizeof(int));					//num objects
		for (int i = 0; i < (int)_objects.size(); i++) {
			out.write((char*)&_objects[i]._type, sizeof(int));			//type
			out.write((char*)&_objects[i]._mappos, sizeof(INTPOINT));	//mappos
			out.write((char*)&_objects[i]._meshInstance._pos,sizeof(glm::vec3));			//Pos
			out.write((char*)&_objects[i]._meshInstance._rot,sizeof(glm::vec3));			//Rot
			out.write((char*)&_objects[i]._meshInstance._sca, sizeof(glm::vec3));			//Sca
		}
	}
	out.close();
}

void TERRAIN::LoadTerrain(const char* pfilename)
{
	std::ifstream in(pfilename, std::ios::binary);	//Binary forms

	if (in.good()) {
		Release();		//release all resources

		in.read((char*)&_size, sizeof(INTPOINT));		//read map size

		if (_pMaptiles != nullptr)							//Clear old maptiles
			delete[] _pMaptiles;

		//Create new maptiles
		_pMaptiles = new MAPTILE[_size.x * _size.y];
		memset(_pMaptiles, 0, sizeof(MAPTILE) * _size.x * _size.y);

		//Read the maptile information
		for (int y = 0; y < _size.y; y++) {
			for (int x = 0; x < _size.x; x++) {
				MAPTILE* ptile = GetTile(x, y);
				in.read((char*)&ptile->_type, sizeof(int));			//type
				in.read((char*)&ptile->_height, sizeof(float));		//Height
			}
		}

		//Read number of objects
		int numObjects = 0;
		in.read((char*)&numObjects, sizeof(int));
		for (int i = 0; i < numObjects; i++) {
			int type = 0;
			INTPOINT mp;
			glm::vec3 p, r, s;
			in.read((char*)&type, sizeof(int));						//type
			in.read((char*)&mp, sizeof(INTPOINT));					//mappos
			in.read((char*)&p, sizeof(glm::vec3));					//Pos
			in.read((char*)&r, sizeof(glm::vec3));					//Rot
			in.read((char*)&s, sizeof(glm::vec3));					//Sca

			_objects.push_back(OBJECT(type, mp, p, r, s));
		}

		//Recreate Terrain
		InitPathfinding();
		CreatePatches(3);
		CalculateAlphaMaps();
	}
	in.close();
}
