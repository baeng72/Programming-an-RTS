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
	Renderer::VertexAttributes attributes = { {Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float3,Renderer::ShaderDataType::Float2,Renderer::ShaderDataType::Float2},sizeof(TERRAINVertex) };
	_mesh.reset(Mesh::Mesh::Create(pdevice, (float*)vertices.data(), sizeof(TERRAINVertex) * nrVert, indices.data(), indexCount*sizeof(uint32_t),attributes));
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
}

void TERRAIN::SetWireframe(bool wireframe)
{
	for (auto& shader : _shaders) {
		shader->SetWireframe(wireframe);
	}
	//_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->GetShaderDataByName("FlatDirectional")));
}

void TERRAIN::Init(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager, INTPOINT size_)
{
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.07/textures/grass.jpg")));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.07/textures/mountain.jpg")));
	_diffuseMaps.push_back(std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.07/textures/snow.jpg")));
	_size = size_;
	_shaders.resize(_diffuseMaps.size());//hacktastic, need to a better way  to do this
	for (size_t i = 0; i < _diffuseMaps.size(); i++) {
		if (Core::GetAPI() == Core::API::Vulkan) {
			_shaders[i].reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.07/Shaders/Vulkan/terrain.glsl", false)));
		}
		else {
			_shaders[i].reset(Renderer::Shader::Create(_pdevice, _shaderManager->CreateShaderData("../../../../Resources/Chapter 04/Example 4.07/Shaders/GL/terrain.glsl", false,false)));
			
		}
		
		
	}
	
	//std::vector<Renderer::Texture*> textures{ _diffuseMaps[0].get(),_diffuseMaps[1].get(),_diffuseMaps[2].get() };

	
	_alphaMaps.resize(_diffuseMaps.size());
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
			Rect r = { (int)( x * (_size.x - 1) / (float)numPatches),
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
	constexpr int texWidth = 128;
	constexpr int texHeight = 128;
	//create one alpha map per diffuse map
	uint32_t* pdata = new uint32_t[texWidth*texHeight];
	for (size_t i = 0; i < _diffuseMaps.size(); i++) {
		//for each pixel in the alphaMap
		for (int y = 0; y < texHeight; y++) {
			for (int x = 0; x < texWidth; x++) {
				int hm_x = (int)(_heightMap->_size.x * (x / (float)(texWidth)));
				int hm_y = (int)(_heightMap->_size.y * (y / (float)(texHeight)));
				float height = _heightMap->_pHeightMap[hm_x + hm_y * _heightMap->_size.x];
				if (height >= min_range[i])
					pdata[x + y * texWidth] = 0xFFFFFFFF;
				else
					pdata[x + y * texWidth] = 0;
			}
		}
		//create a new texture
		_alphaMaps[i].reset(Renderer::Texture::Create(_pdevice, texWidth, texHeight, 4, (uint8_t*)pdata));
		if (Core::GetAPI() == Core::API::Vulkan) {
			//what a hack
			Renderer::Texture* pdiffuse = _diffuseMaps[i].get();

			Renderer::Texture* palpha = _alphaMaps[i].get();
			std::vector<Renderer::Texture*> textures = { pdiffuse,palpha };
			_shaders[i]->SetTextures(textures.data(), 2);
		}
	}
	

}

void TERRAIN::Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light)
{
	
	Renderer::FlatShaderDirectionalUBO ubo = { viewProj,light };
	int uboid = 0;
	

	Renderer::FlatShaderPushConst pushConst{model };
	
	
	for (size_t m = 0; m < _diffuseMaps.size(); m++) {
		Renderer::Shader* pshader = _shaders[m].get();
		pshader->Bind();
		if (Core::GetAPI() == Core::API::Vulkan) {
			pshader->SetUniformData("UBO", &ubo, sizeof(ubo));
			pshader->SetPushConstData(&pushConst, sizeof(pushConst));
			//draw for each diffuse map, could do it in one draw call per patch passing a buffer or something?
		}
		else {
			
			
			pshader->SetUniformData("viewProj", &viewProj, sizeof(mat4));
			pshader->SetUniformData("model", &model, sizeof(mat4));
			pshader->SetUniformData("light.ambient", &light.ambient, sizeof(vec4));
			pshader->SetUniformData("light.diffuse", &light.diffuse, sizeof(vec4));
			pshader->SetUniformData("light.specular", &light.specular, sizeof(vec4));
			pshader->SetUniformData("light.direction", &light.direction, sizeof(vec3));
			Renderer::Texture* pdiffuse = _diffuseMaps[m].get();

			Renderer::Texture* palpha = _alphaMaps[m].get();			
			pshader->SetTexture("texmap", &pdiffuse, 1);
			pshader->SetTexture("alphamap", &palpha, 1);
		}
		
		
		for (size_t i = 0; i < _patches.size(); i++)
			_patches[i]->Render();
	}
}
