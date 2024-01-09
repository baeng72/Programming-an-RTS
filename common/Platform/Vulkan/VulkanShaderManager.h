#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <tuple>
#include<functional>
#include "../../Renderer/ShaderManager.h"
#include "VulkanEx.h"
namespace Vulkan {
		
	struct VlkBlock {
		std::string name;
		uint32_t offset;
		uint32_t size;
		uint32_t paddedSize;
		std::vector<VlkBlock> members;
		VlkBlock() {
			offset = size = paddedSize = 0;
		}
	};
	struct VlkImage {
		VkFormat format;
		VkImageType type;
		uint32_t depth;
		uint32_t sampled;
		VlkImage() {
			type = VK_IMAGE_TYPE_MAX_ENUM;
			depth = sampled = 0;
			format = VK_FORMAT_MAX_ENUM;
		}
	};
	enum class VlkResourceType { Undefined = 0, Sampler = 1, CBV = 2, SRV = 4, UAV = 8, PUSH = 16 };
	struct VlkBinding {
		std::string name;
		VlkResourceType restype;
		uint32_t binding;
		uint32_t set;
		uint32_t count;
		VkDescriptorType descriptorType;
		VkShaderStageFlags stageFlags;
		VlkImage image;
		VlkBlock block;
		VlkBinding() {
			restype = VlkResourceType::Undefined;
			binding = set = count = 0;
			descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
			stageFlags = 0;
		}
		VkDescriptorSetLayoutBinding getBinding() {
			VkDescriptorSetLayoutBinding descbind = {};
			descbind.binding = binding;
			descbind.descriptorCount = count;
			descbind.descriptorType = descriptorType;
			descbind.pImmutableSamplers = nullptr;
			descbind.stageFlags = stageFlags;
			return descbind;
		}
		uint32_t getPaddedSize() {
			uint32_t size = 0;
			for (auto& member : block.members) {
				size += member.paddedSize;
			}
			return size;
		}

	};
	struct VlkPushBlock {
		VkShaderStageFlags stageFlags;
		std::string name;
		uint32_t size;
		VlkBlock block;
		VlkPushBlock() {
			stageFlags = 0;
		}
	};
	struct ShaderReflection {
		std::vector<std::vector<VlkBinding>> bindings;
		std::vector<std::tuple<std::string, VkFormat, uint32_t>> inputs;
		VlkPushBlock pushBlock;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t,void*>> blockmembers;//flat list of all resources (name,parentrow,set,binding,offset,size,paddedsize,buffer)
		std::unordered_map<size_t, int> blockmap;//hash of name to row in blockmembers, hashed names could be fully qualified, e.g. UBO.light.diffuse; partially qualified, light.diffuse or simple, diffuse
	};
	struct VulkanShaderData {
		ShaderReflection reflection;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;		
		VkPipelineLayout pipelineLayout;
		VkPipeline		 pipeline;
		VkPipeline		filledPipeline;
		VkPipeline		wireframePipeline;
		VkShaderStageFlags pushConstStages;
		Vulkan::Buffer	uniformBuffer;	
		
		
	
	};
	
	class VulkanShaderManager : public  Renderer::ShaderManager {
		Renderer::RenderDevice* _pdevice;
		
		Vulkan::Buffer _uniformBuffer;
		std::vector<Vulkan::UniformBufferInfo> _uboInfo;		
		std::unordered_map<std::string,VulkanShaderData> _shaderList;
		void CompileShaders();
		void CompileShader(const std::string&name,const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources,bool cullBackFaces,bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes, void* platformData = nullptr);
		std::string readFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& src);
		VkShaderStageFlagBits ShaderTypeFromString(const std::string& type);
		
		void Reflect(std::unordered_map < VkShaderStageFlagBits, std::vector<uint32_t>>& spirvMap, ShaderReflection&reflection);
	public:		
		VulkanShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~VulkanShaderManager();
		virtual void* GetShaderDataByName(const char*pname) override;
		virtual void* CreateShaderData(const char* shaderPath,bool cullBackFaces=true,bool enableBlend=true,bool enableDepth=true,Renderer::ShaderStorageType * ptypes = nullptr, uint32_t numtypes=0, void* platformData = nullptr) override;				
	};
}
