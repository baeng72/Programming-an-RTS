#include "terrain.h"
#include "../../../common/Platform/Vulkan/VulkanEx.h"
#include "../../../common/Platform/Vulkan/VulkState.h"
#include "../../../common/Platform/Vulkan/VulkSwapchain.h"
#include "../../../common/Platform/Vulkan/ShaderCompiler.h"

PATCH::PATCH()
{
	_pdevice = nullptr;
	
}

PATCH::~PATCH()
{
	Release();
}

bool PATCH::CreateMesh(HEIGHTMAP& hm, Rect source, Renderer::RenderDevice* pdevice, int index,void*shaderData, std::vector<std::shared_ptr<Renderer::Texture>>& textures)
{
	_pdevice = pdevice;
	int width = source.right - source.left;
	int height = source.bottom - source.top;
	int nrVert = (width + 1) * (height + 1);
	int nrTri = width * height * 2;
	std::vector<TERRAINVertex> vertices(nrVert);
	for (int z = source.top, z0 = 0; z <= source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x <= source.right; x++, x0++) {
			//Calculate vertex color
			glm::vec2 uv = glm::vec2(x * 0.2f, z * 0.2f);
			//Extract height (and position) from heightMap
			glm::vec3 pos = glm::vec3(x, hm._pHeightMap[x + z * hm._size.x], -z);

			vertices[z0 * (width + 1) + x0] = TERRAINVertex(pos,uv);
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
	std::vector<uint32_t> attr(nrTri);//attribute buffer
	int a = 0;
	for (int z = source.top, z0 = 0; z < source.bottom; z++, z0++) {
		for (int x = source.left, x0 = 0; x < source.right; x++, x0++) {
			uint32_t subset = 0;
			if (hm._pHeightMap[x + z * hm._size.x] == 0.f)
				subset = 0;
			else if (hm._pHeightMap[x + z * hm._size.x] <= hm._maxHeight * 0.6f)
				subset = 1;
			else
				subset = 2;
			attr[a++] = subset;
			attr[a++] = subset;
		}
	}
	_attrBuffer.reset(Renderer::Buffer::Create(_pdevice, attr.data(), (uint32_t)attr.size() * sizeof(uint32_t)));
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
	_mesh.reset(Renderer::Mesh::Create(pdevice, (float*)vertices.data(), sizeof(TERRAINVertex) * nrVert, indices.data(), indexCount*sizeof(uint32_t)));
	_shader.reset(Renderer::Shader::Create(pdevice, shaderData));// _shaderManager->GetShaderData("../../../../Resources/Chapter 04/Example 4.05/Shaders/FlatDirectional2.glsl")));
	int colorid = 0;
	_shader->SetStorage(colorid, _attrBuffer.get());
	std::vector<Renderer::Texture*> tex;
	for (auto& t : textures)
	{
		tex.push_back(t.get());
	}
	_shader->SetTexture(colorid, tex.data(), (uint32_t)tex.size());
	return false;
}

void PATCH::Release() {
	_mesh.reset();
}

void PATCH::Render(glm::mat4&viewProj,glm::mat4&model, Renderer::DirectionalLight& light)
{
	Renderer::FlatShaderDirectionalUBO ubo = { viewProj,light };
	int uboid = 0;
	_shader->SetUniformData("UBO", &ubo, sizeof(ubo));

	Renderer::FlatShaderPushConst pushConst{ model };
	_shader->SetPushConstData(&pushConst, sizeof(pushConst));
	
	_mesh->Render(_shader.get());
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
	for (int i = 0; i < _patches.size(); i++) {
		_patches[i]->SetWireframe(wireframe);
	}
	
	//_shader.reset(Renderer::Shader::Create(_pdevice, _shaderManager->GetShaderDataByName("FlatDirectional")));
}

void TERRAIN::Init(Renderer::RenderDevice* pdevice,std::shared_ptr<Renderer::ShaderManager> shaderManager, INTPOINT size_)
{
	_pdevice = pdevice;
	_shaderManager = shaderManager;
	_textures.push_back(std::shared_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.05/images/water.jpg")));
	_textures.push_back(std::shared_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.05/images/grass.jpg")));
	_textures.push_back(std::shared_ptr<Renderer::Texture>(Renderer::Texture::Create(pdevice, "../../../../Resources/Chapter 04/Example 4.05/images/stone.jpg")));
	_size = size_;	
	
	GenerateRandomTerrain(3);
}

void TERRAIN::GenerateRandomTerrain(int numPatches)
{
	Release();

	//Create two heightmaps and multiply them

	_heightMap = std::make_unique<HEIGHTMAP>(_size, 15.f);
	HEIGHTMAP hm2(_size, 30.f);

	_heightMap->CreateRandomHeightMap(rand() % 2000, 2.5f, 0.5f, 8);
	hm2.CreateRandomHeightMap(rand() % 2000, 2.5f, 0.7f, 3);
	hm2.Cap(hm2._maxHeight * 0.4f);

	*_heightMap *= hm2;

	CreatePatches(numPatches);
	
}

void TERRAIN::CreatePatches(int numPatches)
{
	_pdevice->Wait();//who needs synchronisation when you can block GPU?
	void* shaderData = _shaderManager->GetShaderData("../../../../Resources/Chapter 04/Example 4.05/Shaders/TexturedDirectional.glsl");
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
			p->CreateMesh(*_heightMap, r, _pdevice, x + y,shaderData,_textures);
			_patches.push_back(p);
		}
	}
}

void TERRAIN::Render(glm::mat4&viewProj,glm::mat4&model,Renderer::DirectionalLight&light)
{
	/*Renderer::FlatShaderDirectionalUBO ubo = { viewProj,light };
	int uboid = 0;
	_shader->SetUniformData("UBO",&ubo, sizeof(ubo));

	Renderer::FlatShaderPushConst pushConst{model };
	
	_shader->SetPushConstData(&pushConst,sizeof(pushConst));*/
	for (size_t i = 0; i < _patches.size(); i++)
		_patches[i]->Render(viewProj,model,light);
}
