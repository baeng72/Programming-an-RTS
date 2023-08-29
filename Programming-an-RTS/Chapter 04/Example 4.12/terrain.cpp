#include "terrain.h"


PATCH::PATCH()
{
	_pdevice = nullptr;
	
}

PATCH::~PATCH()
{
	Release();
}

bool PATCH::CreateMesh(HEIGHTMAP& hm, Rect source, Renderer::RenderDevice* pdevice)
{
	_pdevice = pdevice;
	int width = source.right - source.left;
	int height = source.bottom - source.top;
	int nrVert = (width + 1) * (height + 1);
	int nrTri = width * height * 2;
	float invSizeX = 1 / (float)hm._size.x;
	float invSizeY = 1 / (float)hm._size.y;
	std::vector<TERRAINVertex> vertices(nrVert);
	for (int z = source.top, z0 = 0; z <= source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x <= source.right; x++, x0++) {
			//Strect UV coordinates once over the entire terrain
			
			glm::vec2 alphaUV = glm::vec2(x * invSizeX, z * invSizeY);
			glm::vec2 uv = alphaUV * 8.f;
			//Extract height (and position) from heightMap
			
			glm::vec3 pos = glm::vec3(x, hm._pHeightMap[x + z * hm._size.x], -z);

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
	
		_mesh.reset(Mesh::Mesh::Create(pdevice, (float*)vertices.data(), sizeof(TERRAINVertex) * nrVert, indices.data(), indexCount * sizeof(uint32_t)));
	
	
	return false;
}

void PATCH::Release() {
	_mesh.reset();
}

void PATCH::Render(Renderer::Shader* pshader)
{

	_mesh->Render(pshader);
}


TERRAIN::TERRAIN()
{
	_pMaptiles = nullptr;
	_pdevice = nullptr;
	
}
void TERRAIN::Cleanup()
{
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
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.12/textures/grass.jpg")));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.12/textures/mountain.jpg")));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.12/textures/snow.jpg")));
	
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.12/Shaders/terrain.glsl",false)));

	GenerateRandomTerrain(3);
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
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
			p->CreateMesh(*_heightMap, r, _pdevice);
			_patches.push_back(p);
		}
	}
	
}

void TERRAIN::CalculateAlphaMaps() {
	//height ranges
	float min_range[] = { 0.f,1.f,15.f };
	float max_range[] = { 2.f,16.f,21.f };
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
				int hm_x = (int)(_heightMap->_size.x * (x / (float)(texWidth)));
				int hm_y = (int)(_heightMap->_size.y * (y / (float)(texHeight)));
				float height = _heightMap->_pHeightMap[hm_x + hm_y * _heightMap->_size.x];
				uint32_t b = 0;
				if (height >= min_range[i] && height < max_range[i])
					b = 0xFF;
				else
					b = 0;
				b <<= shift;
				pdata[x + y * texWidth] |= b;
			}
		}
		
	}
	//create a new texture
	_alphaMap.reset(Renderer::Texture::Create(_pdevice, texWidth, texHeight, 4, (uint8_t*)pdata));
	std::vector<Renderer::Texture*> textures = { _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get(),_alphaMap.get() };
	_shader->SetTextures(textures.data(), 4);

}

void TERRAIN::Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light)
{
	Renderer::FlatShaderDirectionalUBO ubo = { viewProj,light };
	int uboid = 0;
	

	Renderer::FlatShaderPushConst pushConst{model };
	
	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));
	_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	
	
	for (size_t i = 0; i < _patches.size(); i++)
		_patches[i]->Render(_shader.get());
	

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
			ptile->_height = _heightMap->GetHeight(x, y);

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
}

MAPTILE* TERRAIN::GetTile(int x, int y)
{
	return &_pMaptiles[x + y * _size.x];
}
