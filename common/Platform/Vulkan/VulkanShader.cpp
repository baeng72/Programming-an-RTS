#pragma once
#include <glm/glm.hpp>
#include "VulkanShader.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "VulkanBuffer.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"
#include <cstring>
Renderer::Shader* Renderer::Shader::Create(Renderer::RenderDevice* pdevice, void* shaderData) {
	return new Vulkan::VulkanShader(pdevice, shaderData);
}
namespace Vulkan {

	VulkanShader::VulkanShader(Renderer::RenderDevice* pdevice, void* shaderData) {
		_pShaderData = (VulkanShaderData*)shaderData;
		_pdevice = pdevice;

		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		auto& layouts = _pShaderData->descriptorSetLayouts;
		auto& sets = _descriptorSets;
		sets.resize(layouts.size(), VK_NULL_HANDLE);
		//allocate descriptor sets
		for (size_t set = 0; set < layouts.size(); set++) {
			VkDescriptorSet descriptorSet;
			context.pPoolCache->allocateDescriptorSet(&descriptorSet, layouts[set]);
			sets[set] = descriptorSet;
		}
		uint32_t index = 0;
		VkDeviceSize offset = 0;
		auto& uboSetBindings = _pShaderData->uboSetBindings;
		auto& uniformBuffer = _pShaderData->uniformBuffer;
		//need a descriptor set
		if (uboSetBindings.size() > 0) {
			uint32_t set = uboSetBindings[0] >> 16;
			std::vector<VkDescriptorBufferInfo> bufferInfos(16);
			auto uniupdated = DescriptorSetUpdater::begin(context.pLayoutCache, layouts[set], sets[set]);
			for (auto& uboBinding : uboSetBindings) {
				uint32_t set = uboBinding >> 16;
				uint32_t binding = uboBinding & 0xFF;
				
				
				bufferInfos[binding].buffer = uniformBuffer.buffer;
				bufferInfos[binding].offset = offset;
				
				auto& name = _pShaderData->uboNames[index];
				VkDeviceSize size = _pShaderData->uboSizeMap[name];
				bufferInfos[binding].range = size;
				index++;
				offset += size;

				uniupdated.AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[binding]);

				
			}
			uniupdated.update();
			
		}
		

		
	}
	Vulkan::VulkanShader::~VulkanShader()
	{
		/*Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::cleanupPipeline(context.device, pipeline);
		Vulkan::cleanupPipelineLayout(context.device, pipelineLayout);*/
		
	}
	
	void VulkanShader::Bind(uint32_t* pdynoffsets, uint32_t dynoffcount)
	{

		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;

		vkCmdBindDescriptorSets(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipelineLayout, 0, (uint32_t)_descriptorSets.size(), _descriptorSets.data(), dynoffcount, pdynoffsets);
		vkCmdBindPipeline(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipeline);
	}

	void VulkanShader::SetPushConstData(void* pdata, uint32_t size) {
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;

		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, 0, size, pdata);

	}

	void VulkanShader::SetWireframe(bool wireframe)
	{
		_pShaderData->pipeline = wireframe ? _pShaderData->wireframePipeline : _pShaderData->filledPipeline;
	}

	bool VulkanShader::SetUniformBuffer(uint32_t i, Renderer::Buffer* pbuffer,bool dynamic)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VulkanBufferData* pdata = (VulkanBufferData*)pbuffer->GetNativeHandle();
		//need to update descriptor
		if (i >= 0) {
			uint32_t setBinding = dynamic ? _pShaderData->uniformDynamicSetBindings[i] : _pShaderData->uboSetBindings[i];
			uint32_t set = setBinding >> 16;
			uint32_t binding = setBinding & 0x00FF;
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = pdata->buffer.buffer;
			bufferInfo.offset = 0;	//may not be true ?
			bufferInfo.range = pdata->size;
			DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set])
				.AddBinding(binding, pdata->descriptorType, &bufferInfo)
				.update();
			if (dynamic) {
				_pShaderData->uniformDynamicMap[_pShaderData->uniformDynamicNames[i]] = pdata->ptr;
				_pShaderData->uniformDynamicSizeMap[_pShaderData->uniformDynamicNames[i]] = pdata->size;
			}
			else {
				_pShaderData->uboMap[_pShaderData->uboNames[i]] = pdata->ptr;//use an array and index using id?
				_pShaderData->uboSizeMap[_pShaderData->uboNames[i]] = pdata->size;
			}
			return true;
		}
		return false;
	}

	bool VulkanShader::SetUniformBuffer(const char* pname, Renderer::Buffer* pbuffer,bool dynamic)
	{
		uint32_t id = GetUniformId(pname,dynamic);
		return SetUniformBuffer(id, pbuffer,dynamic);
	}

	bool VulkanShader::SetUniformData(uint32_t i, void*ptr,uint32_t len,bool dynamic)
	{
		if (dynamic) {
			assert(i < _pShaderData->uniformDynamicNames.size());
			if (i < _pShaderData->uniformDynamicNames.size()) {
				auto& name = _pShaderData->uniformDynamicNames[i];
				VkDeviceSize size = _pShaderData->uniformDynamicSizeMap[name];
				void* pdst = _pShaderData->uniformDynamicMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;
			}
		}
		else {
			assert(i < _pShaderData->uboNames.size());
			if (i < _pShaderData->uboNames.size()) {
				auto& name = _pShaderData->uboNames[i];
				VkDeviceSize size = _pShaderData->uboSizeMap[name];
				void* pdst = _pShaderData->uboMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;
			}
		}
		return false;
	}

	bool VulkanShader::SetUniformData(const char* pname, void* ptr, uint32_t len,bool dynamic)
	{

		std::string name = pname;
		if (dynamic) {
			auto& names = _pShaderData->uniformDynamicNames;
			ASSERT(std::find(names.begin(), names.end(), name) != names.end(), "Unknown shader name!");
			if (_pShaderData->uniformDynamicMap.find(name) != _pShaderData->uniformDynamicMap.end()) {
				VkDeviceSize size = _pShaderData->uniformDynamicSizeMap[name];
				void* pdst = _pShaderData->uniformDynamicMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;

			}
		}
		else {
			auto& names = _pShaderData->uboNames;
			ASSERT(std::find(names.begin(), names.end(), name) != names.end(), "Unknown shader name!");
			if (_pShaderData->uboMap.find(name) != _pShaderData->uboMap.end()) {
				VkDeviceSize size = _pShaderData->uboSizeMap[name];
				void* pdst = _pShaderData->uboMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;

			}
		}
		return false;
	}

	
	uint32_t VulkanShader::GetUniformId(const char* pname,bool dynamic)
	{
		auto& names = dynamic ? _pShaderData->uniformDynamicNames : _pShaderData->uboNames;
		ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown uniform name!");		
		return (uint32_t)std::distance(names.begin(),std::find(names.begin(), names.end(), pname));
	}

	bool VulkanShader::SetTexture(uint32_t id, Renderer::Texture** pptexture, uint32_t count)
	{
		size_t hash = 0;
		for (size_t i = 0; i < count; i++) {
			Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();			
			hash ^= HASH(pvtext);
		}
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VkDescriptorSet descriptor = _textureDescriptors[hash];
		uint32_t setBinding = _pShaderData->imageSetBindings[id];
		uint32_t set = setBinding >> 16;
		uint32_t binding = setBinding & 0x00FF;
		uint32_t imageCount = _pShaderData->imageCounts[id];
		if (descriptor == VK_NULL_HANDLE) {
			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
				.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, imageCount)
				.build(descriptor);
			_textureDescriptors[hash] = descriptor;


			if (id < _pShaderData->imageNames.size()) {
				ASSERT(count == imageCount, "Incorrect texture count!");
				std::vector<VkDescriptorImageInfo>imageInfos(count);

				for (uint32_t i = 0; i < count; i++) {
					Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();
					imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfos[i].imageView = pvtext->imageView;
					imageInfos[i].sampler = pvtext->sampler;

				}
				DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], descriptor)
					.AddBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfos.data(), count)
					.update();

			
			}
		}
		if (descriptor != VK_NULL_HANDLE) {
			_descriptorSets[set] = descriptor;
			return true;
		}
		return false;
	}

	bool VulkanShader::SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count)
	{
		uint32_t id = GetTextureId(pname);
		return SetTexture(id, pptexture,count);
	}

	bool VulkanShader::SetTextures(Renderer::Texture** pptextures, uint32_t count)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		ASSERT(count == _pShaderData->imageNames.size(), "Invalid number of textures!");
		
		
			
		if (count <= _pShaderData->imageNames.size()) {
			auto setBindings = _pShaderData->imageSetBindings;
			std::vector<std::vector<uint32_t>> groupedSetBindings(_pShaderData->descriptorSetLayouts.size());//in case they are all over the shop
			for (uint32_t i = 0; i < count; i++) {
				uint32_t setbinding = setBindings[i];
				uint32_t set = setbinding >> 16;
				uint32_t binding = setbinding & 0xFF;
				groupedSetBindings[set].push_back(binding);
			}
			uint32_t index = 0;
			for (size_t set = 0; set < groupedSetBindings.size(); set++) {
				auto& groupedBindings = groupedSetBindings[set];
				if (groupedBindings.size() == 0)
					continue;
				size_t hash = 0;
				for (size_t b = 0; b < groupedBindings.size(); b++) {
					Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
					hash ^= HASH(ptextdata);
				}
				index = 0;
				VkDescriptorSet descriptor = _textureDescriptors[hash];
				if (descriptor == VK_NULL_HANDLE) {
					auto setbuilder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
					for (size_t b = 0; b < groupedBindings.size(); b++) {

						setbuilder.AddBinding(groupedBindings[b], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
					}
					setbuilder.build(descriptor);
					_textureDescriptors[hash] = descriptor;
					auto setupdater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], descriptor);

					std::vector<VkDescriptorImageInfo> imageInfos(groupedBindings.size());
					for (size_t b = 0; b < groupedBindings.size(); b++) {
					
						Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
						imageInfos[b].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfos[b].imageView = ptextdata->imageView;
						imageInfos[b].sampler = ptextdata->sampler;
						setupdater.AddBinding(groupedBindings[b], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfos[b]);
					}
					setupdater.update();
					_textureDescriptors[hash] = descriptor;
				}
				if (descriptor != VK_NULL_HANDLE) {
					_descriptorSets[set] = descriptor;
					return true;
				}
			}
			return true;
		}
		
		
		return false;
	}

	
	uint32_t VulkanShader::GetTextureId(const char* pname)
	{
		auto& names = _pShaderData->imageNames;
		ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown image name!");
		return (uint32_t)std::distance(names.begin(), std::find(names.begin(), names.end(), pname));
	}

	bool VulkanShader::SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer,bool dynamic)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VulkanBufferData* pdata = (VulkanBufferData*)pbuffer->GetNativeHandle();
		//need to update descriptor
		if (i >= 0) {
			uint32_t setBinding = dynamic ? _pShaderData->storageSetDynamicBindings[i] : _pShaderData->storageSetBindings[i];
			uint32_t set = setBinding >> 16;
			uint32_t binding = setBinding & 0x00FF;
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = pdata->buffer.buffer;
			bufferInfo.offset = 0;	//may not be true ?
			bufferInfo.range = pdata->size;
			DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set])
				.AddBinding(binding, pdata->descriptorType, &bufferInfo)
				.update();
			if (dynamic) {
				_pShaderData->storageDynamicMap[_pShaderData->storageDynamicNames[i]] = pdata->ptr;
				_pShaderData->storageDynamicSizeMap[_pShaderData->storageDynamicNames[i]] = pdata->size;
			}
			else {
				 _pShaderData->storageMap[_pShaderData->storageNames[i]] = pdata->ptr;//use an array and index using id?
				_pShaderData->storageSizeMap[_pShaderData->storageNames[i]] = pdata->size;
			}
			return true;
		}
		return false;
	}

	bool VulkanShader::SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer,bool dynamic)
	{
		//need to update descriptor
		uint32_t id = GetStorageId(pname,dynamic);
		return SetStorageBuffer(id, pbuffer,dynamic);		
	}

	bool VulkanShader::SetStorageData(uint32_t i, void* ptr, uint32_t len,bool dynamic)
	{
		if (dynamic) {
			assert(i < (uint32_t)_pShaderData->storageDynamicNames.size());
			if (i < (uint32_t)_pShaderData->storageDynamicNames.size()) {
				auto& name = _pShaderData->storageDynamicNames[i];
				VkDeviceSize size = _pShaderData->storageDynamicSizeMap[name];
				void* pdst = _pShaderData->storageDynamicMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;
			}
		}
		else {
			assert(i < (uint32_t)_pShaderData->storageNames.size());
			if (i < (uint32_t)_pShaderData->storageNames.size()) {
				auto& name = _pShaderData->storageNames[i];
				VkDeviceSize size = _pShaderData->storageSizeMap[name];
				void* pdst = _pShaderData->storageMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;
			}
		}
		return false;
	}

	bool VulkanShader::SetStorageData(const char* pname, void* ptr, uint32_t len,bool dynamic)
	{
		uint32_t id = GetStorageId(pname,dynamic);
		if (dynamic) {
			if (id < (uint32_t)_pShaderData->storageDynamicNames.size()) {
				SetStorageData(id, ptr, len,dynamic);
				return true;
			}
		}
		else {
			if (id < (uint32_t)_pShaderData->storageNames.size()) {
				SetStorageData(id, ptr, len,dynamic);
				return true;
			}
		}
		return false;
	}

	

	uint32_t VulkanShader::GetStorageId(const char* pname,bool dynamic)
	{
		uint32_t id = UINT32_MAX;
		if (dynamic) {
			auto& names = _pShaderData->storageDynamicNames;
			ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown storage name!");
			id = (uint32_t)std::distance(names.begin(), std::find(names.begin(), names.end(), pname));
		}
		else {
			auto& names = _pShaderData->storageNames;
			ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown storage name!");
			id = (uint32_t)std::distance(names.begin(), std::find(names.begin(), names.end(), pname));
		}
		return id;
	}
	
}
