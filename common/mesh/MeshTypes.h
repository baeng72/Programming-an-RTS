#pragma once
#include <string>
#include <vector>
#include "../Core/defines.h"
namespace Mesh {
	struct Material {
		std::string name;
		vec4 ambient;
		vec4 diffuse;
		vec4 specular;
		vec4 emissive;
		std::vector<std::string> diffuseTextures;
		std::vector<std::string> normalTextures;
		std::vector < std::string> specularTextures;
	};

	struct Primitive {
		std::string name;
		uint32_t vertexStart;		
		uint32_t vertexCount;
		uint32_t indexStart;
		uint32_t indexCount;		
		uint32_t materialIndex;
	};

	struct AnimationChannel {
		std::vector<float> positionTimes;
		std::vector<vec3> positions;
		std::vector<float> rotationTimes;
		std::vector<quat> rotations;
		std::vector<float> scaleTimes;
		std::vector<vec3> scales;
	};
	struct AnimationClip {
		std::string name;
		float ticksPerSecond;
		std::vector<AnimationChannel> channels;
	};
	struct Skeleton {
		std::vector<int> boneHierarchy;
		std::vector<std::string> boneNames;
		std::vector<mat4> boneInvBindMatrices;
		std::vector<mat4> bonePoseMatrices;
	};
	enum class MeshType { position, position_normal, position_normal_uv, pos_norm_uv_bones, pos_norm_uv_tan, pos_norm_uv_tan_bones };
	enum class TextureType { diffuse };
	
}