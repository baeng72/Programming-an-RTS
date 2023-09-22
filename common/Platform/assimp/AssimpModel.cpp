#pragma once
#include <unordered_map>
#include "AssimpModel.h"
#include "../../Core/Log.h"

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
			Mesh::Skeleton skel;
			skel.boneNames = _boneNames;
			skel.boneHierarchy = _boneHierarchy;
			skel.boneInvBindMatrices = _boneOffsets;
			skel.bonePoseMatrices = _boneXForms;
			_skeletons.push_back(skel);
			//do skinning vertex stuff
			{
				for (unsigned int i = 0; i < pscene->mNumMeshes; i++) {
					auto &primitive = _primitives[i];
					auto pmesh = pscene->mMeshes[i];
					auto numBones = pmesh->mNumBones;
					
					for (unsigned int boneId = 0; boneId < numBones; boneId++) {
						aiBone* pbone = pmesh->mBones[boneId];
						auto name = pbone->mName.C_Str();
						auto boneIndex = std::distance(_boneNames.begin(), std::find(_boneNames.begin(), _boneNames.end(), name));
						//assign any bone weights and stuff
						unsigned int boneWeightCount = pbone->mNumWeights;
						auto weights = pbone->mWeights;
						for (unsigned int boneWeightID = 0; boneWeightID < boneWeightCount; ++boneWeightID) {
							auto vertexId = weights[boneWeightID].mVertexId;
							if (vertexId == 0) {
								int z = 0;
							}
							float weight = weights[boneWeightID].mWeight;
							AssimpVertex& vert = primitive.vertices[vertexId];
							for (int i = 0; i < 4; ++i) {
								if (vert.boneIDs[i] < 0) {
									vert.boneIDs[i] = (int)boneIndex;
									vert.weights[i] = weight;
									break;
								}
							}
						}
					}
				}
			}
			
			ProcessAnimations(pscene);
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

	vec3 AssimpModel::AssimpToGLM(aiVector3D& vec) {
		return vec3(vec.x, vec.y, vec.z);
	}
	quat AssimpModel::AssimpToGLM(aiQuaternion& qu) {
		return quat(qu.w, qu.x, qu.y, qu.z);
	}

	void AssimpModel::ProcessMaterials(const aiScene* pscene)
	{
		for (uint32_t i = 0; i < pscene->mNumMaterials; i++) {
			Mesh::Material mat;
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
			float tx=0.f, ty=0.f, tz=0.f;
			float btx = 0.f, bty = 0.f, btz = 0.f;
			if (pmesh->HasTangentsAndBitangents()) {
				//tangent
				tx = pmesh->mTangents[i].x;
				ty = pmesh->mTangents[i].y;
				tz = pmesh->mTangents[i].z;
				//bitangent
				btx = pmesh->mBitangents[i].x;
				bty = pmesh->mBitangents[i].y;
				btz = pmesh->mBitangents[i].z;
			}


			glm::vec3 norm = -1.f * glm::normalize(glm::vec3(nx, ny, nz));
			verts[i] = AssimpVertex(glm::vec3(x, y, z), norm, glm::vec2(ux, uy),vec3(tx,ty,tz),vec3(btx,bty,btz),ivec4(-1),vec4(0.f));
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
		
		
		//std::unordered_map<std::string, mat4> offsetMap;
		for (unsigned int boneId = 0; boneId < numBones; boneId++) {
			aiBone* pbone = pmesh->mBones[boneId];
			mat4 offset = AssimpToGLM(pbone->mOffsetMatrix);
			auto name = pbone->mName.C_Str();
			_boneMap[name] = offset;
			
		}
		
	}
	void AssimpModel::ProcessTextureTypes(aiMaterial* pmat, aiTextureType type, std::vector<std::string>& textureNames)
	{
		uint32_t texCount = pmat->GetTextureCount(type);
		for (uint32_t i = 0; i < texCount; i++) {
			aiString str;
			pmat->GetTexture(type, i, &str);
			std::string filename = str.C_Str();
			auto pos = filename.find(".dds");
			if (pos != std::string::npos) {
				filename.replace(pos, 4, ".png");
			}
			textureNames.push_back(filename);
		}
	}

	void AssimpModel::ProcessAnimations(const aiScene* pscene)
	{
		unsigned int animationCount = pscene->mNumAnimations;
		_animations.resize(animationCount);
		for (unsigned int aniIdx = 0; aniIdx < animationCount; ++aniIdx) {
			auto panimation = pscene->mAnimations[aniIdx];
			const std::string name = panimation->mName.C_Str();
			_animations[aniIdx].name = name;
			float duration = (float)panimation->mDuration;
			float ticksPerSecond = (float)panimation->mTicksPerSecond > 0.0f ? (float)panimation->mTicksPerSecond : 30.0f;
			_animations[aniIdx].ticksPerSecond = ticksPerSecond;
			unsigned int channelCount = panimation->mNumChannels;
			_animations[aniIdx].channels.resize(_boneCount);//assimp doesn't load bones that have no vertices attached, so ignore?
			for (unsigned int chIdx = 0; chIdx < channelCount; ++chIdx) {
				
				auto pchannel = panimation->mChannels[chIdx];
				const std::string chname = pchannel->mNodeName.data;
				if (std::find(_boneNames.begin(), _boneNames.end(),chname) == _boneNames.end())
					continue;
				size_t channelId = std::distance(_boneNames.begin(), std::find(_boneNames.begin(), _boneNames.end(), chname));
				auto& dstchannel = _animations[aniIdx].channels[channelId];//this may not be correct channel (bone), need to compare name with hierarchy
				unsigned int positionCount = pchannel->mNumPositionKeys;
				dstchannel.positions.resize(positionCount);
				dstchannel.positionTimes.resize(positionCount);
				for (unsigned int p = 0; p < positionCount; ++p) {
					float time = (float)pchannel->mPositionKeys[p].mTime;
					vec3 position = AssimpToGLM(pchannel->mPositionKeys[p].mValue);
					dstchannel.positions[p] = position;
					dstchannel.positionTimes[p] = time;
				}
				unsigned int rotationCount = pchannel->mNumRotationKeys;
				dstchannel.rotations.resize(rotationCount);
				dstchannel.rotationTimes.resize(rotationCount);
				for (unsigned int r = 0; r < rotationCount; ++r) {
					float time = (float)pchannel->mRotationKeys[r].mTime;
					quat rotation = AssimpToGLM(pchannel->mRotationKeys[r].mValue);
					dstchannel.rotations[r] = rotation;
					dstchannel.rotationTimes[r] = time;
				}
				unsigned int scaleCount = pchannel->mNumScalingKeys;
				dstchannel.scales.resize(scaleCount);
				dstchannel.scaleTimes.resize(scaleCount);
				for (unsigned int s = 0; s < scaleCount; ++s) {
					float time = (float)pchannel->mScalingKeys[s].mTime;
					vec3 scale = AssimpToGLM(pchannel->mScalingKeys[s].mValue);
					dstchannel.scales[s] = scale;
					dstchannel.scaleTimes[s] = time;
				}
				
			}
		}
	}


	void AssimpModel::ProcessBoneHierarchy(AssimpNode& node, int parentID) {
		if (_boneMap.find(node.name) != _boneMap.end()) {
			int boneID = _boneCount++;
			_boneHierarchy[boneID] = parentID;
			_boneXForms[boneID] = node.localXForm;
			_boneNames[boneID] = node.name;
			_boneOffsets[boneID] = _boneMap[node.name];
			
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
		else {
			//unused bone
			int z = 0;
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
		else if (meshType == Mesh::MeshType::position_normal_uv) {
			struct PosNormUV {
				vec3 pos;
				vec3 norm;
				vec2 uv;
			};
			std::vector<PosNormUV> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUV) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::Mesh::Create(_pdevice, (float*)vertices.data(),sizeof(PosNormUV), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_bones) {
			struct PosNormUVBones {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				ivec4 bones;
				vec4 weights;
			};
			std::vector<PosNormUVBones> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].boneIDs,_primitives[i].vertices[v].weights };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVBones) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::Mesh::Create(_pdevice, (float*)vertices.data(), sizeof(PosNormUVBones), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_tan) {
			struct PosNormUVTan {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				vec3 tangent;
				vec3 bitangent;
			};
			std::vector<PosNormUVTan> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].tangent,_primitives[i].vertices[v].bitangent };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVTan) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::Mesh::Create(_pdevice, (float*)vertices.data(), sizeof(PosNormUVTan), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_tan_bones) {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::Mesh::Create(_pdevice, (float*)_primitives[i].vertices.data(), sizeof(AssimpVertex), vertSize, (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		return nullptr;
	}
	Mesh::AnimatedMesh* AssimpModel::GetAnimatedMesh(Mesh::MeshType meshType, uint32_t i)
	{
		auto skel = _skeletons[i];
		auto animations = _animations;
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
			return Mesh::AnimatedMesh::Create(_pdevice, (float*)vertices.data(), vertSize,sizeof(PosNorm), (uint32_t*)_primitives[i].indices.data(), indSize,skel,animations);
		}
		else if (meshType == Mesh::MeshType::position_normal_uv) {
			struct PosNormUV {
				vec3 pos;
				vec3 norm;
				vec2 uv;
			};
			std::vector<PosNormUV> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUV) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::AnimatedMesh::Create(_pdevice, (float*)vertices.data(), vertSize,sizeof(PosNormUV), (uint32_t*)_primitives[i].indices.data(), indSize, skel, animations);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_bones) {
			struct PosNormUVBones {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				ivec4 bones;
				vec4 weights;
			};
			std::vector<PosNormUVBones> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].boneIDs,_primitives[i].vertices[v].weights };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVBones) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::AnimatedMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNormUVBones), (uint32_t*)_primitives[i].indices.data(), indSize, skel, animations);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_tan) {
			struct PosNormUVTan {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				vec3 tangent;
				vec3 bitangent;
			};
			std::vector<PosNormUVTan> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].tangent,_primitives[i].vertices[v].bitangent};
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVTan) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::AnimatedMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNormUVTan), (uint32_t*)_primitives[i].indices.data(), indSize, skel, animations);
		}
		else {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::AnimatedMesh::Create(_pdevice, (float*)_primitives[i].vertices.data(), vertSize, sizeof(AssimpVertex), (uint32_t*)_primitives[i].indices.data(), indSize, skel, animations);
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
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNorm), (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::position_normal_uv) {
			struct PosNormUV {
				vec3 pos;
				vec3 norm;
				vec2 uv;
			};
			std::vector<PosNormUV> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUV) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNormUV), (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_bones) {
			struct PosNormUVBones {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				ivec4 bones;
				vec4 weights;
			};
			std::vector<PosNormUVBones> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].boneIDs,_primitives[i].vertices[v].weights };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVBones) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNormUVBones), (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else if (meshType == Mesh::MeshType::pos_norm_uv_tan) {
			struct PosNormUVTan {
				vec3 pos;
				vec3 norm;
				vec2 uv;
				vec3 tangent;
				vec3 bitangent;
			};
			std::vector<PosNormUVTan> vertices(_primitives[i].vertices.size());
			for (size_t v = 0; v < _primitives[i].vertices.size(); v++) {
				vertices[v] = { _primitives[i].vertices[v].position,_primitives[i].vertices[v].normal,_primitives[i].vertices[v].uv,_primitives[i].vertices[v].tangent,_primitives[i].vertices[v].bitangent };
			}
			uint32_t vertSize = (uint32_t)(sizeof(PosNormUVTan) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)vertices.data(), vertSize, sizeof(PosNormUVTan), (uint32_t*)_primitives[i].indices.data(), indSize);
		}
		else {
			uint32_t vertSize = (uint32_t)(sizeof(AssimpVertex) * _primitives[i].vertices.size());
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * _primitives[i].indices.size());
			return Mesh::ProgressiveMesh::Create(_pdevice, (float*)_primitives[i].vertices.data(), vertSize, sizeof(AssimpVertex), (uint32_t*)_primitives[i].indices.data(), indSize);
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
	uint32_t AssimpModel::GetTextureCount(uint32_t matId,Mesh::TextureType type)
	{
		if (type == Mesh::TextureType::diffuse) {
			return (uint32_t)_materials[matId].diffuseTextures.size();
		}
		return 0;
	}
	Renderer::Texture* AssimpModel::GetTexture(uint32_t matId,Mesh::TextureType type, uint32_t i)
	{
		if (type == Mesh::TextureType::diffuse) {
			std::string textpath = _path + _materials[matId].diffuseTextures[i];
			return Renderer::Texture::Create(_pdevice, textpath.c_str());
		}
		return nullptr;
	}
	uint32_t AssimpModel::GetMaterialCount()
	{
		return (uint32_t)_materials.size();
	}
	Mesh::Material* AssimpModel::GetMaterial(uint32_t i)
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
	void AssimpModel::GetBoneInvBindXForms(uint32_t i, std::vector<mat4>& boneInvBindXForms) {
		boneInvBindXForms = _boneOffsets;
	}
	void AssimpModel::GetBoneHierarchy(uint32_t i, std::vector<int>& boneHierarchy)
	{
		boneHierarchy = _boneHierarchy;
	}

	uint32_t AssimpModel::GetAnimationCount(uint32_t i)
	{
		return (uint32_t)_animations.size();
	}

	void AssimpModel::GetAnimation(uint32_t i, uint32_t aniIdx, Mesh::AnimationClip& animation)
	{
		animation = _animations[aniIdx];
	}


	Mesh::MultiMesh* AssimpModel::GetMultiMesh(std::shared_ptr<Renderer::ShaderManager>& shaderManager) {
		//put together all the vertices
		struct PosNormUV {
			vec3 pos;
			vec3 norm;
			vec2 uv;
		};
		std::vector<PosNormUV> vertices;
		std::vector<uint32_t> indices;
		std::vector<Mesh::Primitive> primitives;
		uint32_t vertexStart = 0;
		
		uint32_t vertexStride = sizeof(PosNormUV);
		uint32_t indexStart = 0;
		uint32_t indexCount = 0;
		for (auto& prim : _primitives) {
			Mesh::Primitive primitive;
			primitive.vertexStart = vertexStart;
			primitive.vertexCount = (uint32_t)prim.vertices.size();
			primitive.indexStart = indexStart;
			primitive.indexCount = (uint32_t)prim.indices.size();
			primitive.materialIndex = prim.materialIndex;
			primitive.name = prim.name;
			primitives.push_back(primitive);
			for (auto& vert : prim.vertices) {
				
				PosNormUV v = { vert.position,vert.normal,vert.uv };
				vertices.push_back(v);				
			}
			for (auto& ind : prim.indices) {
				indices.push_back(indexStart + ind);
			}
			vertexStart += (uint32_t)prim.vertices.size();
			indexStart += (uint32_t)prim.indices.size();
		}
		std::vector<float> fvertices(vertices.size() * sizeof(PosNormUV));
		memcpy(fvertices.data(), vertices.data(), vertices.size() * sizeof(PosNormUV));
		std::vector<std::unique_ptr<Renderer::Texture>> textures(_materials.size());
		for (size_t i = 0; i < _materials.size();i++) {
			std::string textpath = _path + _materials[i].diffuseTextures[0];
			textures[i] = std::unique_ptr<Renderer::Texture>(Renderer::Texture::Create(_pdevice, textpath.c_str()));
		}
		return Mesh::MultiMesh::Create(_pdevice,shaderManager, _xform, fvertices, indices, vertexStride, primitives, textures);
	}
	
}
