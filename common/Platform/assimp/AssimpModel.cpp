#pragma once
#include <unordered_map>
#include "AssimpModel.h"
#include "../../Core/Log.h"

#include "../../anim/pose.h"
#include "../../anim/clip.h"
//#define __OPTIMIZE__
#ifdef __OPTIMIZE__
#include "../meshoptimizer/src/meshoptimizer.h"
#endif
namespace Mesh {
	Model* Model::Create(Renderer::RenderDevice* pdevice, const char* pmodelPath) {
		return new Assimp::AssimpModel(pdevice, pmodelPath);
	}
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
			
			_boneCount = 0;
			ProcessBoneHierarchy(_rootNode, -1);
			
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
			Mesh::ModelMaterial mat;
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
	void AssimpModel::ProcessNode(aiNode* pnode, const aiScene* pscene, mat4&parentXForm,AssimpNode* pcurrentNode) {
		AssimpNode& currentNode = *pcurrentNode;
		glm::mat4 localXForm = AssimpToGLM(pnode->mTransformation);
		glm::mat4 nodeXForm = parentXForm * localXForm;

		AssimpNode node{ pnode->mName.C_Str(),localXForm,nodeXForm };
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

		//uint32_t indCount = (uint32_t)inds.size();
#ifdef __OPTIMIZE__
		uint32_t vertStride = sizeof(AssimpVertex), vertCount = (uint32_t) verts.size();
		float* pvertices = (float*)verts.data();
		uint32_t indCount = (uint32_t)inds.size();
		uint32_t* pindices = inds.data();
		std::vector<uint32_t> indices(pindices, pindices + indCount);

		std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
		size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, vertStride);
		std::vector<unsigned int> newIndices(indCount);
		meshopt_remapIndexBuffer(newIndices.data(), pindices, indCount, remap.data());
		std::vector<float> newVertices(vertStride * vertCount);
		meshopt_remapVertexBuffer(newVertices.data(), pvertices, vertCount, vertStride, remap.data());
		inds = newIndices;
		for (uint32_t i = 0; i < vertCount; i++) {
			AssimpVertex* ptr = reinterpret_cast<AssimpVertex*>(newVertices.data() + i * sizeof(AssimpVertex));
			verts[i] = *ptr;
		}
		
#endif
		AssimpPrimitive primitive = { pmesh->mName.C_Str(), verts,inds,pmesh->mMaterialIndex };
		_primitives.push_back(primitive);
		_materialIndices.push_back(pmesh->mMaterialIndex);
		
		auto numBones = pmesh->mNumBones;
		//_nameList.resize(numBones);
		_boneNames.resize(numBones);
		_boneOffsets.resize(numBones);
		_boneHierarchy.resize(numBones);
		_boneXForms.resize(numBones);
		_bones.resize(numBones);
		
		//std::unordered_map<std::string, mat4> offsetMap;
		for (unsigned int boneId = 0; boneId < numBones; boneId++) {
			aiBone* pbone = pmesh->mBones[boneId];
			mat4 offset = AssimpToGLM(pbone->mOffsetMatrix);
			auto name = pbone->mName.C_Str();
			_boneMap[name] = offset;
			//offsetMap[name] = offset;
			//_nameList[boneId] = name;
			//_boneOffsets[boneId] = offset;
		}
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


	void AssimpModel::ProcessBoneHierarchy(AssimpNode& node, int parentID) {
		if (_boneMap.find(node.name) != _boneMap.end()) {
			int boneID = _boneCount++;
			_boneHierarchy[boneID] = parentID;
			_boneXForms[boneID] = node.localXForm;
			_boneNames[boneID] = node.name;
			_boneOffsets[boneID] = _boneMap[node.name];
			_bones[boneID] = mat4ToTransform(node.localXForm);
			/*mat4 offset = _boneMap[node.name];
			mat4 invoffset = glm::inverse(offset);
			mat4 xform = node.nodeXForm;
			mat4 invxform = glm::inverse(xform);
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (offset[i][j] != invxform[i][j]) {
						int z = 0;
					}
				}
			}*/
			parentID = boneID;
			
		}
		for (auto child : node.children) {
			ProcessBoneHierarchy(child, parentID);
		}
	}

	uint32_t AssimpModel::GetMeshCount()
	{
		return (uint32_t)_primitives.size();
	}
	Mesh::Mesh* AssimpModel::GetMesh(Mesh::MeshType meshType,uint32_t i)
	{
		if (meshType == Mesh::MeshType::position_normal) {
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
			return Mesh::Mesh::Create(_pdevice, (float*)vertices.data(),sizeof(PosNorm), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		if (meshType == Mesh::MeshType::position_normal_uv) {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::Mesh::Create(_pdevice, (float*)_primitives[i].vertices.data(),sizeof(AssimpVertex), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		return nullptr;
	}
	Mesh::ProgressiveMesh* AssimpModel::GetProgressiveMesh(Mesh::MeshType meshType, uint32_t i)
	{
		if (meshType == Mesh::MeshType::position_normal) {
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
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)vertices.data(), vertSize,sizeof(PosNorm), (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		if (meshType == Mesh::MeshType::position_normal_uv) {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)_primitives[i].vertices.data(), vertSize,sizeof(AssimpVertex), (uint32_t*)_primitives[i].indices.data(), indSize);
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
	uint32_t AssimpModel::GetTextureCount(Mesh::TextureType type)
	{
		if (type == Mesh::TextureType::diffuse) {
			return (uint32_t)_diffuseTextures.size();
		}
		return 0;
	}
	Renderer::Texture* AssimpModel::GetTexture(Mesh::TextureType type, uint32_t i)
	{
		if (type == Mesh::TextureType::diffuse) {
			std::string textpath = _path + _diffuseTextures[i];
			return Renderer::Texture::Create(_pdevice, textpath.c_str());
		}
		return nullptr;
	}
	uint32_t AssimpModel::GetMaterialCount()
	{
		return (uint32_t)_materials.size();
	}
	Mesh::ModelMaterial* AssimpModel::GetMaterial(uint32_t i)
	{
		return &_materials[i];
	}

	uint32_t AssimpModel::GetBoneCount(uint32_t i)
	{
		return _boneCount;
	}

	void AssimpModel::GetBoneNames(uint32_t i, std::vector<std::string>& boneNames)
	{
		boneNames = _boneNames;
	}

	void AssimpModel::GetBoneXForms(uint32_t i, std::vector<mat4>& boneXForms)
	{
		boneXForms = _boneXForms;
	}

	void AssimpModel::GetBoneHierarchy(uint32_t i, std::vector<int>& boneHierarchy)
	{
		boneHierarchy = _boneHierarchy;
	}
	
}
