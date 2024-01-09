#include "terrain.h"
#include "camera.h"
#include "mapObject.h"
#include "player.h"
#include <../ThirdParty/glad/include/glad/glad.h>
#include "../common/Platform/GL/GLERR.h"
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

	_mapRect = source;

	_BBox.min = vec3(10000.f);
	_BBox.max = vec3(-10000.f);

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
			MAPTILE* ptile = t.GetTile(x, z);
			glm::vec3 pos = glm::vec3(x, ptile->_height, -z);
			glm::vec2 alphaUV = glm::vec2(x * invSizeX, z * invSizeY);
			glm::vec2 uv = alphaUV * 8.f;
			//Extract height (and position) from heightMap
			
			

			vertices[z0 * (width + 1) + x0] = TERRAINVertex(pos,t.GetNormal(x,z), uv,alphaUV);

			//Calculate bounding box bounds...
			_BBox.min.x = std::min(_BBox.min.x, pos.x);
			_BBox.min.y = std::min(_BBox.min.y, pos.y);
			_BBox.min.z = std::min(_BBox.min.z, pos.z);

			_BBox.max.x = std::max(_BBox.max.x, pos.x);
			_BBox.max.y = std::max(_BBox.max.y, pos.y);
			_BBox.max.z = std::max(_BBox.max.z, pos.z);
		}
	}
	std::vector<uint32_t> indices(3 * nrTri);
	uint32_t indexCount = 0;
	for (int z = source.top, z0 = 0; z < source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x < source.right; x++, x0++) {
			//Triangle 1
			indices[indexCount++] = z0 *		(width + 1) + x0;
			indices[indexCount++] = z0 *		(width + 1) + x0 + 1;
			indices[indexCount++] = (z0 + 1) *	(width + 1) + x0;
			//Triangle 2
			indices[indexCount++] = (z0 + 1) *	(width + 1) + x0;
			indices[indexCount++] = z0 *		(width + 1) + x0 + 1;
			indices[indexCount++] = (z0 + 1) *	(width + 1) + x0 + 1;
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
	_vertices.resize(vertices.size());
	for (size_t i = 0; i < vertices.size();i++) {
		_vertices[i] = vertices[i].position;
	}
	_indices = indices;
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
	if(_pdevice)
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

void TERRAIN::Init(Renderer::RenderDevice* pdevice,Core::Window*pwindow, std::shared_ptr<Renderer::ShaderManager> shaderManager, INTPOINT size_)
{
	_firstFogOfWar = true;
	_fogOverride = false;
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	_size = size_;
	_updateSight = true;

	if (_pMaptiles != nullptr)//clear old maptiiles
		delete[] _pMaptiles;
	//Create maptiles
	_pMaptiles = new MAPTILE[_size.x * _size.y];
	memset(_pMaptiles, 0, sizeof(MAPTILE) * _size.x * _size.y);

	//Load Textures
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("grass.jpg"))));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("mountain.jpg"))));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, Core::ResourcePath::GetTexturePath("snow.jpg"))));
	
	//_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 09/Example 9.03/Shaders/terrain.glsl",false)));

	
	_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("terrain.glsl"), false)));
	_objectShader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("mesh.glsl"))));
		
	InitFogOfWar();

	InitLandscape();

	InitMinimap();

	_font.reset(Renderer::Font::Create());
	_font->Init(_pdevice, "../../../../Resources/Fonts/arialn.ttf", 40);
	_dirToSun = glm::normalize(vec3(1.f, 0.6f, 0.5f));


	_visibleTiles.resize(_size.x * _size.y);
	_visitedTiles.resize(_size.x * _size.y);


	GenerateRandomTerrain(pwindow, 9);
}

void TERRAIN::InitFogOfWar() {
	{
		constexpr int dim = 64;
		{
			std::vector<uint8_t> pixels(dim * dim);

			memset(pixels.data(), 0, dim * dim);
			constexpr float intensity = 1.3f;
			constexpr float halfdim = (float)dim * 0.5f;
			vec2 centre = vec2(halfdim);
			for (int y = 0; y < dim; y++) {
				for (int x = 0; x < dim; x++) {
					vec2 vec = (centre - vec2(x, y));
					float d = length(vec);
					int32_t value = (int32_t)(((halfdim - d) / halfdim) * 255.f * intensity);
					if (value < 0)
						value = 0;
					if (value > 255)
						value = 255;
					pixels[x + y * dim] = (uint8_t)value;

				}
			}

			_sightTexture.reset(Renderer::Texture::Create(_pdevice, dim, dim, Renderer::TextureFormat::R8, pixels.data()));
			_sightTexture->SetName("sightTexture");
		}
		{
			struct SIGHTVertex {
				vec3 position;
				vec2 uv;
				SIGHTVertex(vec3 pos, vec2 tx) {
					position = pos;
					uv = tx;
				}
			};
			std::vector<SIGHTVertex> vertices = {
				SIGHTVertex(vec3(-1.f, 0.f, 1.f), vec2(0.f)),
				SIGHTVertex(vec3(1.f, 0.f, 1.f), vec2(1.f, 0.f)),
				SIGHTVertex(vec3(-1.f, 0.f, -1.f), vec2(0.f, 1.f)),
				SIGHTVertex(vec3(1.f, 0.f, -1.f), vec2(1.f))
			};
			//create faces
			std::vector<uint32_t> indices = { 0,1,2,1,3,2 };
			Renderer::VertexAttributes attrs{ {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2},sizeof(SIGHTVertex) };
			_sightMesh.reset(Mesh::Mesh::Create(_pdevice, (float*)vertices.data(), (uint32_t)(sizeof(SIGHTVertex) * vertices.size()), indices.data(), (uint32_t)(sizeof(uint32_t) * indices.size()), attrs));
		}

		_visibleTexture.reset(Renderer::Texture::Create(_pdevice, 256, 256, Renderer::TextureFormat::R8G8B8A8));
		_visibleTexture->SetName("visibleTexture");
		auto visibleTexture = _visibleTexture.get();
		_visibleFramebuffer.reset(Renderer::FrameBuffer::Create(_pdevice, &visibleTexture, 1));
		_visibleShader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("visible.glsl"), false, true, false, nullptr, 0, _visibleFramebuffer->GetContext())));
		_visitedTextures.resize(2);
		_visitedTextures[0].reset(Renderer::Texture::Create(_pdevice, 256, 256, Renderer::TextureFormat::R8G8B8A8));
		_visitedTextures[0]->SetName("visitedTexture0");
		_visitedTextures[1].reset(Renderer::Texture::Create(_pdevice, 256, 256, Renderer::TextureFormat::R8G8B8A8));
		_visitedTextures[1]->SetName("visitedTexture1");
		std::vector < Renderer::Texture*> visitedTextures = { _visitedTextures[0].get(), _visitedTextures[1].get() };
		_visitedFramebuffer.reset(Renderer::FrameBuffer::Create(_pdevice, visitedTextures.data(), 2,nullptr, false));
		_visitedShader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("visited.glsl"), false, true, false, nullptr, 0, _visitedFramebuffer->GetContext())));
		_fowTexture.reset(Renderer::Texture::Create(_pdevice, 256, 256, Renderer::TextureFormat::R8G8B8A8));
		_fowTexture->SetName("FogOfWarTexture");
		auto fowTexture = _fowTexture.get();
		_fowFramebuffer.reset(Renderer::FrameBuffer::Create(_pdevice, &fowTexture, 1));
		_fowShader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("fogofwar.glsl"), false, true, false, nullptr, 0, _fowFramebuffer->GetContext())));

	}
}

void TERRAIN::InitLandscape() {
	{
		//white lightmap texture
		constexpr int dim = 32;
		std::vector<uint8_t> pixels(dim * dim,255);
		_whiteLightmap.reset(Renderer::Texture::Create(_pdevice, dim, dim, Renderer::TextureFormat::R8, pixels.data()));
		_whiteLightmap->SaveToFile(Core::ResourcePath::GetTexturePath("whitelightmap.jpg"));
	}
	
	{
		//framebuffer rendertarget texture
		constexpr int dim = 256;
		_landscapeTexture.reset(Renderer::Texture::Create(_pdevice, dim, dim, Renderer::TextureFormat::R8G8B8A8));
	}
	{
		//landscape depth map
		constexpr int dim = 256;
		_landscapeDepthMap.reset(Renderer::Texture::Create(_pdevice, dim, dim, Renderer::TextureFormat::D32));
	}
	{

		//framebuffer
		Renderer::Texture* textures[1] = { _landscapeTexture.get() };
		_landscapeFramebuffer.reset(Renderer::FrameBuffer::Create(_pdevice, textures, 1,_landscapeDepthMap.get(), true,true));
	}
}

void TERRAIN::InitMinimap() {
	{
		//minimap border texture
		_minimapBorder.reset(Renderer::Texture::Create(_pdevice, Core::ResourcePath::GetTexturePath("minimap.png")));
	}
	{
		//minimap render texture
		constexpr int dim = 256;
		_minimapTexture.reset(Renderer::Texture::Create(_pdevice, dim, dim, Renderer::TextureFormat::R8G8B8A8));
	}
	{
		//minimap framebuffer
		Renderer::Texture* textures[1] = { _minimapTexture.get() };
		_minimapFramebuffer.reset(Renderer::FrameBuffer::Create(_pdevice, textures, 1));
	}
	{
		_minimapShader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData(Core::ResourcePath::GetShaderPath("minimap.glsl"), false, true, false, nullptr, 0, _minimapFramebuffer->GetContext())));
	}
	{
		_minimapSprite.reset(Renderer::Sprite::Create(_pdevice));
		
	}
}

void TERRAIN::GenerateRandomTerrain(Core::Window* pwindow, int numPatches)
{
	Release();

	//Create two heightmaps and multiply them

	_heightMap = std::make_unique<HEIGHTMAP>(_size, 20.f);
	HEIGHTMAP hm2(_size, 2.f);
	HEIGHTMAP hm3(_size, 2.f);

	_heightMap->CreateRandomHeightMap(rand() % 2000, 2.0f, 0.7f, 8);
	hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.8f, 3);
	hm2.Cap(hm2._maxHeight * 0.4f);

	//load 4 player filter
	hm3.LoadFromFile("../../../../Resources/Chapter 09/Example 9.03/heightmaps/four_players.jpg");


	*_heightMap *= hm2;
	*_heightMap *= hm3;

	
	//Add Objects
	HEIGHTMAP hm4(_size, 1.f);
	hm4.CreateRandomHeightMap(rand() % 1000, 5.5f, 0.9f, 7);

	for (int y = 0; y < _size.y; y++) {
		for (int x = 0; x < _size.x; x++) {
			if (_heightMap->GetHeight(x, y) == 0.f && hm4.GetHeight(x, y) > 0.7f && rand() % 6 == 0)
				AddObject(0, INTPOINT(x, y));	//tree
			else if (_heightMap->GetHeight(x, y) >= 1.f && hm4.GetHeight(x, y) > 0.9f && rand() % 20 == 0)
				AddObject(1, INTPOINT(x, y));//stone
		}
	}
	
	
	InitPathfinding();
		
		
	CreatePatches(numPatches);
		
		
	CalculateAlphaMaps();
		
	CalculateLightMap(pwindow);


	RenderLandscape();
	
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
		Progress("Creating Terrain Mesh", y / (float)numPatches);
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
	LOG_INFO("Terrain: Calculating AlphaMaps...");
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
	_alphaMap->SaveToFile(Core::ResourcePath::GetTexturePath("alphamap.jpg"));
	/*std::vector<Renderer::Texture*> textures = { _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get(),_alphaMap.get() };
	_shader->SetTextures(textures.data(), 4);*/
	delete[] pdata;

}

void TERRAIN::CalculateLightMap(Core::Window* pwindow)
{
	
		LOG_INFO("Terrain: Calclating LightMap...");
		constexpr int LMAP_DIM = 256;
		uint8_t* map = new uint8_t[LMAP_DIM * LMAP_DIM];
		memset(map, 255, LMAP_DIM * LMAP_DIM);
		for (int y = 0; y < LMAP_DIM; y++) {

			Progress("Calculating Lightmap", y / (float)LMAP_DIM);
			for (int x = 0; x < LMAP_DIM; x++) {
				float terrain_x = (float)_size.x * (x / (float)LMAP_DIM);
				float terrain_z = (float)_size.y * (y / (float)LMAP_DIM);
				pwindow->OnUpdate();//poll events
				//Find patch that the terrain_x, terrain_z is over
				bool done = false;
				for (int p = 0; p < _patches.size() && !done; p++) {
					Rect mr = _patches[p]->_mapRect;

					//Focus within patch maprect or not?
					if (terrain_x >= mr.left && terrain_x < mr.right &&
						terrain_z >= mr.top && terrain_z < mr.bottom) {
						//Collect only the closest intersection
						RAY rayTop(glm::vec3(terrain_x, 10000.f, -terrain_z), glm::vec3(0.f, -1.f, 0.f));
						float dist = rayTop.Intersect(_patches[p]->_vertices, _patches[p]->_indices);
						if (dist >= 0.f) {
							RAY ray(vec3(terrain_x, 10000.f - dist + 0.01f, -terrain_z), _dirToSun);
							for (int p2 = 0; p2 < _patches.size() && !done; p2++) {
								if (ray.Intersect(_patches[p2]->_BBox) >= 0) {
									if (ray.Intersect(_patches[p2]->_vertices, _patches[p2]->_indices) >= 0.f) {//in shadow
										done = true;
										map[y * LMAP_DIM + x] = 128;
									}
								}
							}
							done = true;
						}
					}
				}
			}

		}
		//Smooth lightmap
		for (int i = 0; i < 3; i++) {

			Progress("Smoothing the LightMap", i / 3.f);
			uint8_t* tmpBytes = new uint8_t[LMAP_DIM * LMAP_DIM];
			memcpy(tmpBytes, map, LMAP_DIM * LMAP_DIM);

			for (int y = 1; y < LMAP_DIM - 1; y++) {
				pwindow->OnUpdate();//poll events
				for (int x = 1; x < LMAP_DIM - 1; x++) {
					long index = y * LMAP_DIM + x;
					uint8_t b1 = map[index];
					uint8_t b2 = map[index - 1];
					uint8_t b3 = map[index - LMAP_DIM];
					uint8_t b4 = map[index + 1];
					uint8_t b5 = map[index + LMAP_DIM];

					tmpBytes[index] = (uint8_t)((b1 + b2 + b3 + b4 + b5) / 5);
				}
			}
			memcpy(map, tmpBytes, LMAP_DIM * LMAP_DIM);
			delete[] tmpBytes;
		}
		_lightMap.reset(Renderer::Texture::Create(_pdevice, LMAP_DIM, LMAP_DIM, Renderer::TextureFormat::R8, (uint8_t*)map));
		_lightMap->SaveToFile(Core::ResourcePath::GetProjectResourcePath("textures/lightmap.jpg"));
		delete[] map;
	
}

void TERRAIN::Progress(const char*ptext, float prc)
{
	if (prc < 0.01f)
		prc = 0.01f;
	_pdevice->StartRender();
	Rect rc = { 200,250,600,300 };
	float width, height;
	_font->GetTextSize(ptext, width, height);
	_font->Draw(ptext, (int)(rc.left+rc.Width() / 2 - width / 2), (int)(rc.top+rc.Height() / 2 - height / 2), Color(0.f, 0.f, 0.f, 1.f));
	_font->Render();
	//Progress Bar
	Rect r;
	r = { 200,300,600,340 };
	_pdevice->Clear(r, Color(0.f, 0.f, 0.f, 1.f));
	r = { 202,302,598,338 };
	_pdevice->Clear(r, Color(1.f));
	r = { 202,302,202 + (int)(396 * prc),338 };
	_pdevice->Clear(r, Color(0.f, 1.f, 0.f, 1.f));

	_pdevice->EndRender();
}

void TERRAIN::Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light,CAMERA&camera)
{
	light.direction = _dirToSun;
	vec2 mapSize = vec2(_size.x, _size.y);
	//_shader->Bind();
	
	_shader->SetUniform("viewProj", &viewProj);
	_shader->SetUniform("model", &model);
	_shader->SetUniform("light.ambient", &light.ambient);
	_shader->SetUniform("light.diffuse", &light.diffuse);
	_shader->SetUniform("light.specular", &light.specular);
	_shader->SetUniform("light.direction", &light.direction);
	Renderer::Texture* plightmap = _lightMap.get();
	Renderer::Texture* pfogOfWar = _fowTexture.get();
	Renderer::Texture* plight = _fogOverride ? plightmap : pfogOfWar;
	std::vector<Renderer::Texture*> textures = { _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get(),_alphaMap.get(),plight };
	
	_shader->SetTextures(textures.data(), 5);
	
	_shader->Bind();//update descriptors if required
	for (size_t i = 0; i < _patches.size(); i++)
		_patches[i]->Render();
	
	_objectShader->SetUniform("viewProj", &viewProj);
	//_objectShader->SetUniform("model", &model);
	_objectShader->SetUniform("light.ambient", &light.ambient);
	_objectShader->SetUniform("light.diffuse", &light.diffuse);
	_objectShader->SetUniform("light.specular", &light.specular);
	_objectShader->SetUniform("light.direction", &light.direction);
	_objectShader->SetUniform("mapSize", &mapSize);
	
	_objectShader->SetTexture("lightmap", &plight, 1);
	//_objectShader->Bind();
	//render object
	for (int i = 0; i < _objects.size(); i++) {
		if (!camera.Cull(_objects[i]._BBox)) {
			
			_objects[i].Render(_objectShader.get());
		}
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

			if (ptile->_height < 0.3f)
				ptile->_type = 0;		//Grass
			else if (ptile->_height < 7.f)
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

std::vector<INTPOINT> TERRAIN::GetPath(INTPOINT start, INTPOINT goal,bool considerUnits)
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
					if (!considerUnits || pbest->_neighbors[i]->_pMapObject == nullptr) {
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
			}
			pbest->closed = true;							//the best tile has now beed searched, add it to the closed list

		}
	}
	return std::vector<INTPOINT>();		//no path found
	
}

void TERRAIN::UpdatePathfinding(Rect* r)
{
	if (!r) {
		InitPathfinding();
		return;
	}

	//connect maptiles using the neighbors[] pointers
	for (int y = r->top; y <= r->bottom; y++) {
		for (int x = r->left; x <= r->right; x++) {
			MAPTILE* tile = GetTile(x, y);
			if (tile != nullptr && tile->_walkable) {
				//clear old connections
				for (int i = 0; i < 8; i++) {
					tile->_neighbors[i] = nullptr;
				}
				//Possible neighbors
				INTPOINT p[] = { INTPOINT(x - 1, y - 1), INTPOINT(x, y - 1), INTPOINT(x + 1, y - 1),
								INTPOINT(x - 1, y),					  INTPOINT(x + 1, y),
								INTPOINT(x - 1, y + 1), INTPOINT(x, y + 1), INTPOINT(x + 1, y + 1) };

				//For each neighbor
				for (int i = 0; i < 8; i++) {
					if (Within(p[i]))
					{
						MAPTILE* neighbor = GetTile(p[i]);

						//Connect tiles if the neighbor is walkable
						if (neighbor != NULL && neighbor->_walkable)
							tile->_neighbors[i] = neighbor;
					}
				}
			}
		}
	}
	CreateTileSets();
}

MAPTILE* TERRAIN::GetTile(int x, int y)
{
	return &_pMaptiles[x + y * _size.x];
}

vec3 TERRAIN::GetWorldPos(INTPOINT mappos)
{
	if (!Within(mappos))
		return vec3(0.f);
	MAPTILE* tile = GetTile(mappos);
	return vec3(mappos.x, tile->_height, -mappos.y);
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

vec3 TERRAIN::GetNormal(int x, int y) {
	//Neighboring map nodes (D, B, C, F, H, G)
	INTPOINT mp[] = {
		INTPOINT(x-1,y),	INTPOINT(x,y-1),
		INTPOINT(x+1,y-1),	INTPOINT(x+1,y),
		INTPOINT(x,y+1),	INTPOINT(x-1,y+1)
	};

	//if there's an inavlida map node return (0, 1, 0)
	if (!Within(mp[0]) || !Within(mp[1]) || !Within(mp[2]) ||
		!Within(mp[3]) || !Within(mp[4]) || !Within(mp[5]))
		return vec3(0.f, 1.f, 0.f);

	//Calculate the normals of the 6 neighboring planes
	vec3 normal = vec3(0.f);

	for (int i = 0; i < 6; i++) {
		vec3 v1 = GetWorldPos(INTPOINT(x, y));
		vec3 v2 = GetWorldPos(mp[i]);
		vec3 v3 = GetWorldPos(mp[(i + 1) % 6]);
		vec3 norm = glm::cross(v2 - v1, v3 - v1);
		normal += norm;
	}
	return glm::normalize(normal);
}

void TERRAIN::UpdateSightMatrixes(std::vector<MAPOBJECT*>& mapObjects)
{
	memset(_visibleTiles.data(), 0,sizeof(uint8_t) * _size.x * _size.y);
	for (int i = 0; i < mapObjects.size(); i++) {
		if (mapObjects[i]) {
			int sr = (int)mapObjects[i]->_sightRadius;
			int a = 0;
			INTPOINT start = mapObjects[i]->_mappos - INTPOINT(sr, sr);
			INTPOINT end = mapObjects[i]->_mappos + INTPOINT(sr, sr);

			for (int y = start.y; y <= end.y; y++) {
				for (int x = start.x; x <= end.x; x++) {
					if (x >= 0 && y >= 0 && x < _size.x && y < _size.y) {
						int index = x + y * _size.x;
						_visitedTiles[index] = _visibleTiles[index] = 0xFF;
					}
				}
			}
		}
	}
	//calculate visibility varaible of patches
	for (int i = 0; i < _patches.size(); i++) {
		if (_patches[i]) {
			_patches[i]->_visible = false;
			Rect r = _patches[i]->_mapRect;

			for (int y = r.top; y <= r.bottom && !_patches[i]->_visible; y++) {
				for (int x = r.left; x <= r.right && !_patches[i]->_visible; x++) {
					if (_visitedTiles[x + y * _size.x])
						_patches[i]->_visible = true;
				}
			}
		}
	}
	//Calculate visibility of terrain objects
	for (int i = 0; i < _objects.size(); i++) {
		_objects[i]._visible = _visitedTiles[_objects[i]._mappos.x + _objects[i]._mappos.y * _size.x];
	}

}

void TERRAIN::RenderFogOfWar(PLAYER* player) {
	
	EASY_FUNCTION("RenderFogOfWar");
	_pdevice->StartOffscreenRender();
	{
		EASY_BLOCK("Visible Pass");
		auto& size = _size;
		vec2 centre = vec2((size.x - 1) / 2.f, -(size.y - 1) / 2.f);
		vec3 eye = vec3(centre.x, 1000.f, centre.y);
		vec3 focus = vec3(centre.x, 0.f, centre.y);
		vec3 up = vec3(0.f, 0.f, 1.f);
		mat4 matView = glm::lookAtLH(eye, focus, up);
		mat4 matProj = Core::orthoWH((float)size.x - 1.f, (float)size.y - 1.f, .1f, 2000.f);
		mat4 matVP = matProj * matView;
		Renderer::ClearColor clrs[1] = { vec4(0.f) };
		if (_fogOverride)
			clrs[0] = { vec4(0.13f,0.13f,0.13f,1.f) };
		
		
		
		_visibleFramebuffer->SetClearColor(clrs, 1);
		_visibleFramebuffer->StartRender();
		Renderer::Texture* sightMap = _sightTexture.get();

		_visibleShader->SetTexture("sightTexture", &sightMap, 1);
		_visibleShader->Bind();//can bind here, as we're not updating any buffers or textures		
		_visibleShader->SetUniform("matVP", matVP);
		_sightMesh->Bind();

		mat4 matWorld;
		for (int u = 0; u < player->_mapObjects.size(); u++) {
			if (player->_mapObjects[u]) {
				auto mapObject = player->_mapObjects[u];
				matWorld = mapObject->GetWorldMatrix();
				vec3 pos = matWorld[3];
				
				vec3 scale = vec3(mapObject->_sightRadius, 1.f, mapObject->_sightRadius);
				matWorld = glm::scale(glm::translate(mat4(1.f), pos), scale);
				_visibleShader->SetUniform("matWorld", matWorld);		//this is either a push constant or memory in a buffer we've set			
				_sightMesh->Render();
			}
		}
		

		_visibleFramebuffer->EndRender();
	}
	{
		EASY_BLOCK("Visited Pass");
		auto visibleTexture = _visibleTexture.get();
		auto visibleTexture2 = _visibleFramebuffer->GetTexture();
		visibleTexture->SaveToFile(Core::ResourcePath::GetTexturePath("visible.jpg"));
		_visitedFramebuffer->StartRender();
		auto visitedTexture = _visitedFramebuffer->GetTexture();
		
		_visitedShader->SetTexture("visibleTexture", &visibleTexture, 1);
		_visitedShader->SetTexture("visitedTexture", &visitedTexture, 1);
		_visitedShader->Bind();
		_visitedFramebuffer->DrawVertices(6);
		_visitedFramebuffer->EndRender();

	}
	{
		EASY_BLOCK("Fog Of War Pass");
		auto lightMap = _lightMap.get();
		auto visibleTexture = _visibleTexture.get();
		auto visitedTexture = _visitedFramebuffer->GetTexture();
		_fowShader->SetTexture("visibleTexture", &visibleTexture, 1);
		_fowShader->SetTexture("visitedTexture", &visitedTexture, 1);
		_fowShader->SetTexture("lightMap", &lightMap, 1);
		_fowShader->Bind();
		_fowFramebuffer->StartRender();
		_fowFramebuffer->DrawVertices(6);
		_fowFramebuffer->EndRender();

	}
	_pdevice->EndOffscreenRender();
	//_fowTexture->SaveToFile(Core::ResourcePath::GetTexturePath("fogofwar.jpg"));

}


void TERRAIN::RenderLandscape() {

	EASY_FUNCTION("RenderLandscape");
	{
		
		auto& size = _size;
		vec2 centre = vec2((size.x - 1) / 2.f, -(size.y - 1) / 2.f);
		vec3 eye = vec3(centre.x, 1000.f, centre.y);
		vec3 focus = vec3(centre.x, 0.f, centre.y);
		vec3 up = vec3(0.f, 0.f, 1.f);
		mat4 matView = glm::lookAtLH(eye, focus, up);
		mat4 matProj = Core::orthoWH((float)size.x - 1.f, (float)size.y - 1.f, .1f, 2000.f);
		mat4 matVP = matProj * matView;
		mat4 model = glm::mat4(1.f);
		Renderer::DirectionalLight light;
		light.direction = _dirToSun;
		light.ambient = vec4(0.5f);
		light.diffuse = vec4(1.f);
		light.specular = vec4(0.5f);

		_pdevice->StartOffscreenRender();
		_landscapeFramebuffer->StartRender();

		vec2 mapSize = vec2(_size.x, _size.y);
		//_shader->Bind();
		{
			EASY_BLOCK("Draw Terrain");
			_shader->SetUniform("viewProj", &matVP);
			_shader->SetUniform("model", &model);
			_shader->SetUniform("light.ambient", &light.ambient);
			_shader->SetUniform("light.diffuse", &light.diffuse);
			_shader->SetUniform("light.specular", &light.specular);
			_shader->SetUniform("light.direction", &light.direction);
			
			
			Renderer::Texture* plightmap = _whiteLightmap.get();
			
			std::vector<Renderer::Texture*> textures = { _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get(),_alphaMap.get(),plightmap };

			_shader->SetTextures(textures.data(), 5);

			_shader->Bind();//update descriptors if required
			for (size_t i = 0; i < _patches.size(); i++)
				_patches[i]->Render();

		}
		{
			_objectShader->SetUniform("viewProj", &matVP);
			//_objectShader->SetUniform("model", &model);
			_objectShader->SetUniform("light.ambient", &light.ambient);
			_objectShader->SetUniform("light.diffuse", &light.diffuse);
			_objectShader->SetUniform("light.specular", &light.specular);
			_objectShader->SetUniform("light.direction", &light.direction);
			_objectShader->SetUniform("mapSize", &mapSize);
			Renderer::Texture* plightmap = _whiteLightmap.get();
			_objectShader->SetTexture("lightmap", &plightmap, 1);
			//_objectShader->Bind();
			//render object
			for (int i = 0; i < _objects.size(); i++) {
					_objects[i].Render(_objectShader.get());				
			}
		}
		_landscapeFramebuffer->EndRender();
		_pdevice->EndOffscreenRender();
	}
	_landscapeTexture->SaveToFile(Core::ResourcePath::GetTexturePath("landscape.jpg"));
	
}

void TERRAIN::UpdateMinimap(std::vector<PLAYER*>&players) {
	EASY_FUNCTION("UpdateMinimap");
	_pdevice->StartOffscreenRender();
	_minimapFramebuffer->StartRender();
	{
		EASY_BLOCK("Render Minimap");
		
		auto landscapeTex = _landscapeTexture.get();
		auto fogOfWarTex = _fowTexture.get();
		_minimapShader->SetTexture("landscapeTexture", &landscapeTex, 1);
		_minimapShader->SetTexture("fogOfWarTexture", &fogOfWarTex, 1);
		_minimapShader->Bind();
		_minimapFramebuffer->DrawVertices(6);
	}
	{
		EASY_BLOCK("Render Players");
		//Draw units and buildings in the player team color
		for (int p = 0; p < players.size(); p++){
			if (players[p] != NULL)
			{
				std::vector<Rect> rects;

				//Get rectangles in "Minimap Space"
				for (int m = 0; m < players[p]->_mapObjects.size(); m++) {
					if (players[p]->_mapObjects[m] != NULL)
						if (!players[p]->_mapObjects[m]->_isBuilding)
						{
							INTPOINT mappos = players[p]->_mapObjects[m]->_mappos;

							//Only add units standing on visible tiles
							if (_visibleTiles[mappos.x + mappos.y * _size.x])
							{
								INTPOINT pos((int)(256.0f * (mappos.x / (float)_size.x)),
									(int)(256.0f * (mappos.y / (float)_size.y)));

								rects.push_back({ pos.x - 1, pos.y - 1,
									pos.x + 2, pos.y + 2 });
							}
						}
						else
						{
							Rect r = players[p]->_mapObjects[m]->GetMapRect(0);

							//Add only those parts of the buildings standing on visited tiles
							for (int y = r.top; y <= r.bottom; y++)
								for (int x = r.left; x <= r.right; x++)
									if (_visitedTiles[x + y * _size.x])
									{
										INTPOINT pos((int)(256.0f * (x / (float)_size.x)),
											(int)(256.0f * (y / (float)_size.y)));

										rects.push_back({ pos.x - 1, pos.y - 1,
											pos.x + 2, pos.y + 2 });
									}
						}
				}
				//Clear rectangles using the team color
				if (!rects.empty())
				{
					Color c = Color((int)(players[p]->_teamColor.x),
						(int)(players[p]->_teamColor.y),
						(int)(players[p]->_teamColor.z),1.f);
					for (auto& r : rects) {
						_minimapFramebuffer->Clear(r, c);
					}
				}
			}
		}		
	}
	_minimapFramebuffer->EndRender();
	_pdevice->EndOffscreenRender();
	_minimapTexture->SaveToFile(Core::ResourcePath::GetTexturePath("minimapout.jpg"));
}

void TERRAIN::RenderMinimap(CAMERA& camera,MOUSE&mouse, Rect& r, Renderer::Line2D* pline) {
	float width = (float)(r.right - r.left);
	float height = (float)(r.bottom - r.top);

	vec2 scalevec = vec2(width / 256.0f,
		height / 256.0f);
	mat4 sca;
	sca = scale(mat4(1.f),vec3(scalevec.x, scalevec.y, 1.0f));
	vec3 vec = vec3(r.left/scalevec.x, r.top/scalevec.y, 0.f);
	_minimapSprite->SetTransform(sca);
	_minimapSprite->Draw(_minimapTexture.get(), vec);
	//Move camera using minimap
	if (mouse.PressInRect(r))
	{
		int x = (int)(((mouse.x - r.left) / width) * _size.x);
		int y = (int)(((mouse.y - r.top) / height) * _size.y);

		camera._focus = GetWorldPos(INTPOINT(x, y));
	}
	//Calculate m_camera frustum viewpoints
	mat4 view, viewInverse;

	view = camera.GetViewMatrix();
	
	int scrw, scrh;
	_pdevice->GetDimensions(&scrw, &scrh);
	//fov_x & fov_y Determines the size of the frustum representation
	float screenRatio = scrh / (float)scrw; 
	float fov_x = 0.4f;
	float fov_y = fov_x * screenRatio;

	//Initialize the four rays
	vec3 org = vec3(0.0f, 0.0f, 0.0f);	//Same origin

	//Four different directions
	vec3 dir[4] = { vec3(-fov_x, fov_y, 1.0f),
						  vec3(fov_x, fov_y, 1.0f),
						  vec3(fov_x, -fov_y, 1.0f),
						  vec3(-fov_x, -fov_y, 1.0f) };
	//Our resulting minimap coordinates
	vec2 points[5];

	//View matrix inverse
	viewInverse = inverse(view);
	
	//D3DXVec3TransformCoord(&org, &org, &viewInverse);
	org = vec3(viewInverse * vec4(org, 1.f));

	//Ground plane
	
	vec3 pt = vec3(0.0f, 0.0f, 0.0f);
	vec3 norm = vec3(0.0f, 1.0f, 0.0f);
	vec4 plane = planeFromPointNormal(pt, norm);

	bool ok = true;

	//check where each ray intersects with the ground plane
	for (int i = 0; i < 4 && ok; i++)
	{
		//Transform ray direction
		dir[i] = vec3(viewInverse * vec4(dir[i], 0.f));
		//D3DXVec3TransformNormal(&dir[i], &dir[i], &viewInverse);
		//D3DXVec3Normalize(&dir[i], &dir[i]);
		dir[i] = normalize(dir[i]);
		dir[i] *= 1000.0f;

		//Find intersection point
		vec3 hit;
		//if (D3DXPlaneIntersectLine(&hit, &plane, &org, &dir[i]) == NULL)
		if(!planeIntersectLine(hit,plane,org,dir[i]))
			ok = false;

		//Make sure the intersection point is on the positive side of the near plane
		//D3DXPLANE n = m_camera.m_frustum[4];
		Plane n = camera._frustum[4];
		float distance = n.a * hit.x + n.b * hit.y + n.c * hit.z + n.d;
		if (distance < 0.0f)ok = false;

		//Convert the intersection point to a minimap coordinate
		if (ok)
		{
			points[i].x = (hit.x / (float)_size.x) * width;
			points[i].y = (-hit.z / (float)_size.y) * height;
		}
	}
	//Set the end point to equal the starting point
	points[4] = points[0];

	//Set viewport to destination rectangle only...
	ViewPort v1, v2;

	v1.x = (float)r.left;
	v1.y = (float)r.top;
	v1.width = (float)width;
	v1.height = (float)height;
	v1.fnear = 0.0f;
	v1.ffar = 1.0f;
	v2.x = v2.y = 0;
	v2.width = (float)scrw;
	v2.height = (float)scrh;
	v2.fnear = 0.f;
	v2.ffar = 1.f;
	_pdevice->SetViewport(v1);

	//Draw camera frustum in the minimap
	if (ok)
	{
		pline->Draw(points, 5, Color(1.f, 1.f, 1.f, 0.25f), 1.f);
		/*m_pLine->SetWidth(1.0f);
		m_pLine->SetAntialias(true);
		m_pLine->Begin();
		m_pLine->Draw(&points[0], 5, 0x44ffffff);
		m_pLine->End();*/
	}

	//Reset viewport
	_pdevice->SetViewport(v2);
	
	//vec3 vec = vec3(r.left / scale.x, r.top / scale.y, 0.0f);
	vec = vec3(800.f - 256.0f, 0.f,0.f);
	_minimapSprite->SetTransform(mat4(1.f));
	_minimapSprite->Draw(_minimapBorder.get(), vec);
}