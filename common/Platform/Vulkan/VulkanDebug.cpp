#include "VulkanDebug.h"
#include <iostream>

namespace Vulkan {
#ifdef ENABLE_DEBUG_MARKER
	
	VulkanDebug::VulkanDebug() {
		active = false;
		pfnDebugMarkerSetObjectTag = VK_NULL_HANDLE;
		pfnDebugMarkerSetObjectName = VK_NULL_HANDLE;
		pfnCmdDebugMarkerBegin = VK_NULL_HANDLE;
		pfnCmdDebugMarkerEnd = VK_NULL_HANDLE;
		pfnCmdDebugMarkerInsert = VK_NULL_HANDLE;
		std::cout << "VulkanDebug::VulkanDebug()" << std::endl;
	}

	VulkanDebug& VulkanDebug::getInstance() {
		static VulkanDebug instance;
		return instance;
	}
	void VulkanDebug::setup(VkDevice device_) {
		device = device_;
		pfnDebugMarkerSetObjectTag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT"));
		std::cout <<"DebugMarkerSetObjectTag: " << std::hex << (pfnDebugMarkerSetObjectTag) << std::endl;
		pfnDebugMarkerSetObjectName = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT"));		
		std::cout << "DebugMarkerSetObjectName: " << std::hex << (pfnDebugMarkerSetObjectName) << std::endl;
		pfnCmdDebugMarkerBegin = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT"));
		pfnCmdDebugMarkerEnd = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT"));
		pfnCmdDebugMarkerInsert = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT"));

		// Set flag if at least one function pointer is present
		active = (pfnDebugMarkerSetObjectName != VK_NULL_HANDLE);
		std::cout << active << std::endl;
	}

	void VulkanDebug::setObjectName(uint64_t object, VkDebugReportObjectTypeEXT objType, const char* pName) {
		//std::cout << "Set object Name: " << pName << " " << std::hex << pfnDebugMarkerSetObjectName << std::endl;
		if (pfnDebugMarkerSetObjectName) {
			VkDebugMarkerObjectNameInfoEXT nameInfo{ VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT };
			nameInfo.objectType = objType;
			nameInfo.object = object;
			nameInfo.pObjectName = pName;
			pfnDebugMarkerSetObjectName(device, &nameInfo);
		}
	}
	void VulkanDebug::setObjectTag(uint64_t object, VkDebugReportObjectTypeEXT objectType, uint64_t name, size_t tagSize, const void* tag) {
		//std::cout << "Set object Tag: " << std::hex << name << " " << pfnDebugMarkerSetObjectTag << std::endl;
		if (pfnDebugMarkerSetObjectTag) {
			VkDebugMarkerObjectTagInfoEXT tagInfo{ VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_TAG_INFO_EXT };
			tagInfo.objectType = objectType;
			tagInfo.object = object;
			tagInfo.tagName = name;
			tagInfo.tagSize = tagSize;
			tagInfo.pTag = tag;
			pfnDebugMarkerSetObjectTag(device, &tagInfo);
		}
	}
	void VulkanDebug::beginMarker(VkCommandBuffer cmdbuffer, const char* pMarkerName, glm::vec4 color) {
		if (pfnCmdDebugMarkerBegin) {
			VkDebugMarkerMarkerInfoEXT markerInfo{ VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
			memcpy(markerInfo.color,&color[0],sizeof(float)*4);
			markerInfo.pMarkerName = pMarkerName;
			pfnCmdDebugMarkerBegin(cmdbuffer, &markerInfo);
		}
	}
	void VulkanDebug::insertMarker(VkCommandBuffer cmdBuffer, const char* pMarkerName, glm::vec4 color) {
		if (pfnCmdDebugMarkerInsert) {
			VkDebugMarkerMarkerInfoEXT markerInfo{ VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT };
			memcpy(markerInfo.color,&color[0], sizeof(float) * 4);
			markerInfo.pMarkerName = pMarkerName;
			pfnCmdDebugMarkerInsert(cmdBuffer, &markerInfo);
		}
	}
	void VulkanDebug::endMarker(VkCommandBuffer cmdBuffer) {
		if (pfnCmdDebugMarkerEnd) {
			pfnCmdDebugMarkerEnd(cmdBuffer);
		}
	}
#endif

}