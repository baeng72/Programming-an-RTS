#pragma once
#include "VulkanMaterial.h"
Renderer::Material* Renderer::Material::Create(Renderer::RenderDevice* pdevice, Renderer::Material::MaterialType type) {
	return new Vulkan::VulkanMaterial(pdevice,type);
}
namespace Vulkan {
	
	VulkanMaterial::VulkanMaterial(Renderer::RenderDevice* pdevice, Renderer::Material::MaterialType type) :_pdevice(pdevice),_type(type){
		_data.descriptorLayout = VK_NULL_HANDLE;
		_data.descriptorSet = VK_NULL_HANDLE;
		_buffer.buffer = VK_NULL_HANDLE;
		VulkContext* pcontext = reinterpret_cast<Vulkan::VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *pcontext;
		switch (type) {
		case Renderer::Material::MaterialType::DiffuseColor:
			BuildDiffuseColor(context);
			break;
		case Renderer::Material::MaterialType::DiffuseColorArray:
			BuildDiffuseColorArray(context);
			break;
		case Renderer::Material::MaterialType::DiffuseTexture:
			BuildDiffuseTexture(context);
			break;
		case Renderer::Material::MaterialType::DiffuseTextureArray:
			BuildDiffuseTextureArray(context);
			break;
		}
	}
	VulkanMaterial::~VulkanMaterial()
	{
		VulkContext* pcontext = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *pcontext;

		if (_bufferInfo.size() || _buffer.buffer != VK_NULL_HANDLE)
			cleanupBuffer(context.device, _buffer);
	}

	void VulkanMaterial::SetAttributes(void* ptr, uint32_t size)
	{
		ASSERT(_type == Renderer::Material::MaterialType::DiffuseColorArray || _type == Renderer::Material::MaterialType::DiffuseTextureArray,"Invalid Material type for attributes.");//only makes sense for these at moemnt
		VulkContext* pcontext = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *pcontext;
		
		UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, true)
			.AddBuffer((VkDeviceSize)size, 1, 1)
			.build(_buffer, _bufferInfo);
		void* pdst = _bufferInfo[0].ptr;
		memcpy(pdst, ptr, size);//copy data
		//update descriptor
		VkDescriptorBufferInfo info{};
		info.buffer = _buffer.buffer;
		info.range = size;
		DescriptorSetUpdater::begin(context.pLayoutCache, _data.descriptorLayout, _data.descriptorSet)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &info)
			.update();
	}

	void VulkanMaterial::SetData(void* ptr, uint32_t size)
	{
		ASSERT(_type == Renderer::Material::MaterialType::DiffuseColor,"Invalid material type data!");
		VulkContext* pcontext = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *pcontext;
		UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true)
			.AddBuffer((VkDeviceSize)size, 1, 1)
			.build(_buffer, _bufferInfo);
		void* pdst = _bufferInfo[0].ptr;
		memcpy(pdst, ptr, size);
		//update descriptor
		VkDescriptorBufferInfo info{};
		info.buffer = _buffer.buffer;
		info.range = size;
		DescriptorSetUpdater::begin(context.pLayoutCache, _data.descriptorLayout, _data.descriptorSet)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &info)
			.update();
	}

	void VulkanMaterial::SetTextures(Renderer::Texture* ptr, uint32_t size)
	{
		ASSERT(_type == Renderer::Material::MaterialType::DiffuseTexture || _type == Renderer::Material::MaterialType::DiffuseTextureArray, "Invalid Material type for textures.");//only makes sense for these at moment
		VulkContext* pcontext = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *pcontext;
		//build array of image infos for update
		std::vector<VkDescriptorImageInfo> imageInfos(size);
		for (size_t i = 0; i < size; i++) {
			Vulkan::Texture* ptext = reinterpret_cast<Vulkan::Texture*>(ptr->GetNativeHandle());
			imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[i].imageView = ptext->imageView;
			imageInfos[i].sampler = ptext->sampler;
		}
		//update correct binding in set
		DescriptorSetUpdater::begin(context.pLayoutCache,_data.descriptorLayout,_data.descriptorSet)
			.AddBinding(_type==Renderer::Material::MaterialType::DiffuseTexture ? 0 : 1,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,imageInfos.data(),size)
			.update();
	}
	void VulkanMaterial::BuildDiffuseColor(VulkContext& context)
	{
		//Simple descriptor set
		//binding=0: ubo with 1 vec4 for color
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_data.descriptorSet, _data.descriptorLayout);

	}
	void VulkanMaterial::BuildDiffuseTexture(VulkContext& context)
	{
		//binding=0: combined image sampler
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_data.descriptorSet, _data.descriptorLayout);
	}
	void VulkanMaterial::BuildDiffuseColorArray(VulkContext& context)
	{
		//binding=0:storage buffer of vec4 for each triangle
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_data.descriptorSet, _data.descriptorLayout);
	}
	void VulkanMaterial::BuildDiffuseTextureArray(VulkContext& context)
	{
		//binding=0: storage buffer indicating triangle's texture index
		//binding=1: texture array		
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(_data.descriptorSet, _data.descriptorLayout);
	}
	
	
}
