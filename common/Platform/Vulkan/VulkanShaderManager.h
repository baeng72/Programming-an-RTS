#pragma once
#include <glm/glm.hpp>
#include <unordered_map>
#include <tuple>
#include<functional>
#include "../../Renderer/ShaderManager.h"
#include "VulkanEx.h"
namespace Vulkan {
	
	//struct VulkanDescriptorData {
	//	VkDescriptorSetLayout descriptorLayout;
	//	VkDescriptorSet descriptor;
	//	VulkanDescriptorData(VkDescriptorSetLayout layout = VK_NULL_HANDLE, VkDescriptorSet set = VK_NULL_HANDLE) {
	//		descriptorLayout = layout;
	//		descriptor = set;
	//	}
	//	/*size_t hash()const {
	//		uint64_t a = reinterpret_cast<uint64_t>(descriptorLayout);
	//		uint64_t b = reinterpret_cast<uint64_t>(descriptor);
	//		uint64_t h = a << 32 | b;
	//		return std::hash<uint64_t>()(h);
	//	}*/
	//};
//}
//namespace std {
//	
//	struct key_hash : public std::unary_function<std::tuple<uint32_t, uint32_t, uint32_t>, std::size_t> {
//		std::size_t operator()(const std::tuple<uint32_t, uint32_t, uint32_t>& k)const {
//			return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
//		}
//	};
//}
//	namespace Vulkan{
	struct VulkanShaderData {
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;		
		VkPipelineLayout pipelineLayout;
		VkPipeline		 pipeline;
		VkPipeline		filledPipeline;
		VkPipeline		wireframePipeline;
		VkShaderStageFlags pushConstStages;
		Vulkan::Buffer	uniformBuffer;		
		std::vector<std::string> uboNames;
		std::vector<uint32_t> uboSetBindings;
		std::unordered_map<std::string, void*> uboMap;
		std::unordered_map<std::string, VkDeviceSize> uboSizeMap;
		std::vector<std::string> storageNames;
		std::vector<uint32_t> storageSetBindings;
		std::unordered_map<std::string, void*>storageMap;
		std::unordered_map<std::string, VkDeviceSize> storageSizeMap;
		std::vector<std::string> imageNames;
		std::vector<uint32_t> imageSetBindings;
		std::vector<uint32_t> imageCounts;
	
	};
	
	class VulkanShaderManager : public  Renderer::ShaderManager {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _uniformBuffer;
		std::vector<Vulkan::UniformBufferInfo> _uboInfo;
		std::unordered_map<std::string,VulkanShaderData> _shaderList;
		//std::unordered_map<std::string,VulkanShaderData> _wireframeShaderList;
		//std::unordered_map<std::tuple<uint32_t,uint32_t,uint32_t>,VulkanDescriptorData,std::key_hash> _shaderAttrMap;
		void CompileShaders();
		void CompileShader(const std::string&name,const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources,bool cullBackFaces,bool enableBlend);
		std::string readFile(const std::string& filepath);
		std::unordered_map<VkShaderStageFlagBits, std::string> PreProcess(const std::string& src);
		VkShaderStageFlagBits ShaderTypeFromString(const std::string& type);
	public:		
		VulkanShaderManager(Renderer::RenderDevice* pdevice);
		virtual ~VulkanShaderManager();
		virtual void* GetShaderDataByName(const char*pname) override;
		virtual void* CreateShaderData(const char* shaderPath,bool cullBackFaces=true,bool enableBlend=true) override;		
		//virtual void* GetShaderAttribute(Renderer::ShaderAttrData& data)override;
		
	};
}
