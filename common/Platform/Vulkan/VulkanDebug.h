#pragma once
#include "Vulkan.h"
#include <glm/glm.hpp>


namespace Vulkan {
#ifdef ENABLE_DEBUG_MARKER
#define MARKER_SETUP(dev) (Vulkan::VulkanDebug::getInstance().setup(dev))
#define BEGIN_MARKER(cmd,text,color) (Vulkan::VulkanDebug::getInstance().beginMarker(cmd,text,color))
#define END_MARKER(cmd)(Vulkan::VulkanDebug::getInstance().endMarker(cmd))
#define NAME_CMD(cmd,name)(Vulkan::VulkanDebug::getInstance().setCommandBufferName(cmd,name))
#define NAME_IMAGE(image,name)(Vulkan::VulkanDebug::getInstance().setImageName(image,name))
#define NAME_SAMPLER(sampler,name)(Vulkan::VulkanDebug::getInstance().setSamplerName(sampler,name))
#define NAME_BUFFER(buffer,name)(Vulkan::VulkanDebug::getInstance().setBufferName(buffer,name))
#define NAME_DESCRIPTOR(desc,name)(Vulkan::VulkanDebug::getInstance().setDescriptorSetName(desc,name))
#define NAME_DESCRIPTOR_LAYOUT(descLay,name)(Vulkan::VulkanDebug::getInstance().setDescriptorSetLayoutName(descLay,name))
#define NAME_PIPELINE(pipe,name)(Vulkan::VulkanDebug::getInstance().setPipelineName(pipe,name))
#define NAME_PIPELINE_LAYOUT(pipeLay,name)(Vulkan::VulkanDebug::getInstance().setPipelineLayoutName(pipeLay,name))
#define NAME_RENDERPASS(rp,name)(Vulkan::VulkanDebug::getInstance().setRenderPassName(rp,name))
#define NAME_FRAMEBUFFER(fb,name)(Vulkan::VulkanDebug::getInstance().setFramebufferName(fb,name))
#define NAME_SEMAPHORE(s,name)(Vulkan::VulkanDebug::getInstance().setSemaphoreName(s,name))
#define NAME_FENCE(f,name)(Vulkan::VulkanDebug::getInstance().setFenceName(f,name))
	class VulkanDebug {
		VkDevice device;
		PFN_vkDebugMarkerSetObjectTagEXT pfnDebugMarkerSetObjectTag;
		PFN_vkDebugMarkerSetObjectNameEXT pfnDebugMarkerSetObjectName;
		PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
		PFN_vkCmdDebugMarkerEndEXT pfnCmdDebugMarkerEnd;
		PFN_vkCmdDebugMarkerInsertEXT pfnCmdDebugMarkerInsert;
		bool active;
		VulkanDebug();
	public:
		static VulkanDebug& getInstance();
		void setup(VkDevice device_);
		bool isActive()const { return active; }
		void setObjectName(uint64_t object, VkDebugReportObjectTypeEXT objType, const char* pName);
		void setObjectTag(uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag);
		void beginMarker(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color);
		void insertMarker(VkCommandBuffer cmdBuffer, const char* pMarkerName, glm::vec4 color);
		void endMarker(VkCommandBuffer);
		void setCommandBufferName(VkCommandBuffer cmdBuffer, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(cmdBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, pName);
		}
		void setQueueName(VkQueue queue, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(queue), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, pName);
		}
		void setImageName(VkImage image, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, pName);
		}
		void setSamplerName(VkSampler sampler, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(sampler), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, pName);
		}
		void setBufferName(VkBuffer buffer, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, pName);
		}
		void setDeviceMemoryName(VkDeviceMemory memory, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(memory), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, pName);
		}
		void setShaderModuleName(VkShaderModule shaderModule, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(shaderModule), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, pName);
		}
		void setPipelineName(VkPipeline pipeline, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, pName);
		}
		void setPipelineLayoutName(VkPipelineLayout pipelineLayout, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(pipelineLayout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, pName);
		}
		void setRenderPassName(VkRenderPass renderPass, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(renderPass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, pName);
		}
		void setFramebufferName(VkFramebuffer frameBuffer, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(frameBuffer), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, pName);
		}
		void setDescriptorSetLayoutName(VkDescriptorSetLayout descriptorSetLayout, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(descriptorSetLayout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, pName);
		}
		void setDescriptorSetName(VkDescriptorSet descriptorSet, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(descriptorSet), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, pName);
		}
		void setSemaphoreName(VkSemaphore semaphore, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(semaphore), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, pName);
		}
		void setFenceName(VkFence fence, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(fence), VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, pName);
		}
		void setEventName(VkEvent event, const char* pName) {
			setObjectName(reinterpret_cast<uint64_t>(event), VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, pName);
		}
	};
#else
#define MARKER_SETUP(dev)
#define BEGIN_MARKER(cmd,text,color) 
#define END_MARKER(cmd)
#define NAME_CMD(cmd,name)
#define NAME_IMAGE(image,name)
#define NAME_SAMPLER(sampler,name)
#define NAME_BUFFER(buffer,name)
#define NAME_DESCRIPTOR(desc,name)
#define NAME_DESCRIPTOR_LAYOUT(descLay,name)
#define NAME_PIPELINE(pipe,name)
#define NAME_PIPELINE_LAYOUT(pipeLay,name)
#define NAME_RENDERPASS(rp,name)
#define NAME_FRAMEBUFFER(fb,name)
#define NAME_SEMAPHORE(s,name)
#define NAME_FENCE(f,name)
#endif
}
