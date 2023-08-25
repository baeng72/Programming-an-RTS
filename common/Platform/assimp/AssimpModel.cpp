#pragma once

#include "AssimpModel.h"
#include "../../Core/Log.h"
Renderer::Model* Renderer::Model::Create(RenderDevice* pdevice, const char *pmodelPath) {
	return new Assimp::AssimpModel(pdevice, pmodelPath);
}
namespace Assimp {
	AssimpModel::AssimpModel(Renderer::RenderDevice* device, const char* pmodelPath)
		:_pdevice(device)
	{
		Assimp::Importer importer;
		const aiScene* pscene{ nullptr };
		//read file
		LOG_INFO("Loading model: {0}", pmodelPath);
		pscene = importer.ReadFile(pmodelPath, aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
		//check for errors
		ASSERT(pscene, "Unable to load model file");;
		ASSERT(!(pscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE),"Unable to load model file");
		ASSERT(pscene->mRootNode,"Unable to load model file");
		if (pscene && !(pscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && pscene->mRootNode) {
			std::string filename = pmodelPath;
			_path = filename.substr(0, filename.find_last_of("/\\") + 1);
			ProcessMaterials(pscene);

			ProcessNode(pscene->mRootNode, pscene, glm::mat4(1.f), &_rootNode);
			_xform = _rootNode.nodeXForm;//probably not true for a lot of models
			
			
		}
		else {
			LOG_CRITICAL("Error loading {0}", pmodelPath);
		}
	}
	AssimpModel::~AssimpModel()
	{
	}
	glm::mat4 AssimpModel::AssimpToGLM(aiMatrix4x4& mat) {
		glm::mat4 gmat = glm::mat4(1.f);
		//glm::mat4 m;
			//memcpy(&m, &mat, sizeof(mat));
			//gmat = glm::transpose(m);
			//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		gmat[0][0] = mat.a1; gmat[1][0] = mat.a2; gmat[2][0] = mat.a3; gmat[3][0] = mat.a4;
		gmat[0][1] = mat.b1; gmat[1][1] = mat.b2; gmat[2][1] = mat.b3; gmat[3][1] = mat.b4;
		gmat[0][2] = mat.c1; gmat[1][2] = mat.c2; gmat[2][2] = mat.c3; gmat[3][2] = mat.c4;
		gmat[0][3] = mat.d1; gmat[1][3] = mat.d2; gmat[2][3] = mat.d3; gmat[3][3] = mat.d4;

		return gmat;
	}
	void AssimpModel::ProcessMaterials(const aiScene* pscene)
	{
		for (uint32_t i = 0; i < pscene->mNumMaterials; i++) {
			Renderer::ModelMaterial mat;
			aiMaterial* pmat = pscene->mMaterials[i];
			aiString name;
			pmat->Get(AI_MATKEY_NAME, name);
			mat.name = name.C_Str();
			aiColor3D ambient;
			pmat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
			mat.ambient = glm::vec4(ambient.r, ambient.g, ambient.b, 1.f);
			aiColor3D diffuse;
			pmat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
			mat.diffuse = glm::vec4(diffuse.r, diffuse.g, diffuse.b, 1.f);
			aiColor3D specular;
			pmat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
			mat.specular = glm::vec4(specular.r, specular.g, specular.b, 1.f);
			aiColor3D emissive;
			pmat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive);
			mat.emissive = glm::vec4(emissive.r, emissive.g, emissive.b, 1.f);
			//get any diffuse textures					
			ProcessTextureTypes(pmat, aiTextureType_DIFFUSE, mat.diffuseTextures);
			_diffuseTextures = mat.diffuseTextures;
			//get any normal textures
			ProcessTextureTypes(pmat, aiTextureType_NORMALS, mat.normalTextures);
			//get any specular textures
			ProcessTextureTypes(pmat, aiTextureType_SPECULAR, mat.specularTextures);
			_materials.push_back(mat);
		}
	}
	void AssimpModel::ProcessNode(aiNode* pnode, const aiScene* pscene, glm::mat4& parentXForm, AssimpNode* pcurrentNode) {
		AssimpNode& currentNode = *pcurrentNode;
		glm::mat4 localXForm = AssimpToGLM(pnode->mTransformation);
		glm::mat4 nodeXForm = localXForm * parentXForm;

		AssimpNode node{ pnode->mName.C_Str(),nodeXForm };
		currentNode = node;

		for (uint32_t i = 0; i < pnode->mNumMeshes; i++) {
			aiMesh* pmesh = pscene->mMeshes[pnode->mMeshes[i]];
			ProcessMesh(pmesh, pscene);
		}
		currentNode.children.resize(pnode->mNumChildren);
		for (uint32_t i = 0; i < pnode->mNumChildren; i++) {
			ProcessNode(pnode->mChildren[i], pscene, currentNode.nodeXForm, &currentNode.children[i]);
		}
	}



	void AssimpModel::ProcessMesh(aiMesh* pmesh, const aiScene* pscene) {

		std::vector<AssimpVertex> verts(pmesh->mNumVertices);
		
		for (uint32_t i = 0; i < pmesh->mNumVertices; i++) {
			float x = pmesh->mVertices[i].x;
			float y = pmesh->mVertices[i].y;
			float z = pmesh->mVertices[i].z;
			float nx = 0.f, ny = 0.f, nz = 0.f;
			if (pmesh->HasNormals()) {
				nx = pmesh->mNormals[i].x;
				ny = pmesh->mNormals[i].y;
				nz = pmesh->mNormals[i].z;
			}
			float ux = 0.f, uy = 0.f;
			if (pmesh->HasTextureCoords(0)) {
				ux = pmesh->mTextureCoords[0][i].x;
				uy = 1.f - pmesh->mTextureCoords[0][i].y;
			}
			glm::vec3 norm = -1.f * glm::normalize(glm::vec3(nx, ny, nz));
			verts[i] = AssimpVertex(glm::vec3(x, y, z), norm, glm::vec2(ux, uy));
		}
		
		
		std::vector<uint32_t> inds(pmesh->mNumFaces * 3);
		for (uint32_t i = 0; i < pmesh->mNumFaces; i++) {
			aiFace face = pmesh->mFaces[i];
			for (uint32_t j = 0; j < face.mNumIndices; j++) {
				inds[i * 3 + j] = face.mIndices[j];
			}
		}

		uint32_t indCount = (uint32_t)inds.size();
		
		AssimpPrimitive primitive = { pmesh->mName.C_Str(), verts,inds,pmesh->mMaterialIndex };
		_primitives.push_back(primitive);
		_materialIndices.push_back(pmesh->mMaterialIndex);
		
	}
	void AssimpModel::ProcessTextureTypes(aiMaterial* pmat, aiTextureType type, std::vector<std::string>& textureNames)
	{
		uint32_t texCount = pmat->GetTextureCount(type);
		for (uint32_t i = 0; i < texCount; i++) {
			aiString str;
			pmat->GetTexture(type, i, &str);
			textureNames.push_back(str.C_Str());
		}
	}
	uint32_t AssimpModel::GetMeshCount()
	{
		return (uint32_t)_primitives.size();
	}
	Renderer::Mesh* AssimpModel::GetMesh(Renderer::MeshType meshType,uint32_t i)
	{
		if (meshType == Renderer::MeshType::position_normal) {
			struct PosNorm {
				glm::vec3 pos;
				glm::vec3 norm;
			};
			std::vector<PosNorm> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNorm) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Renderer::Mesh::Create(_pdevice, (float*)vertices.data(), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		if (meshType == Renderer::MeshType::position_normal_uv) {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Renderer::Mesh::Create(_pdevice, (float*)_primitives[i].vertices.data(), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		return nullptr;
	}
	float* AssimpModel::GetMeshRawVertices(uint32_t i, uint32_t&stride, uint32_t& count)
	{
		stride = sizeof(AssimpVertex);//code using this function must use stride size
		count = (uint32_t)_primitives[i].vertices.size();
		return (float*)_primitives[i].vertices.data();
	}
	uint32_t* AssimpModel::GetMeshRawIndices(uint32_t i, uint32_t& count)
	{
		count = (uint32_t)_primitives[i].indices.size();
		return (uint32_t*)_primitives[i].indices.data();
	}
	glm::mat4 AssimpModel::GetMeshXForm(uint32_t i)
	{
		return _xform;
	}

	uint32_t AssimpModel::GetMeshMaterialIndex(uint32_t i) {
		return _materialIndices[i];
	}
	uint32_t AssimpModel::GetTextureCount(Renderer::TextureType type)
	{
		if (type == Renderer::TextureType::diffuse) {
			return (uint32_t)_diffuseTextures.size();
		}
		return 0;
	}
	Renderer::Texture* AssimpModel::GetTexture(Renderer::TextureType type, uint32_t i)
	{
		if (type == Renderer::TextureType::diffuse) {
			std::string textpath = _path + _diffuseTextures[i];
			return Renderer::Texture::Create(_pdevice, textpath.c_str());
		}
		return nullptr;
	}
	uint32_t AssimpModel::GetMaterialCount()
	{
		return (uint32_t)_materials.size();
	}
	Renderer::ModelMaterial* AssimpModel::GetMaterial(uint32_t i)
	{
		return &_materials[i];
	}
}
