#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Material.h"
#include "../../Renderer/Texture.h"
#include "../../Core/Log.h"
#include "VulkState.h"
#include "VulkanEx.h"
namespace Vulkan {
	//A material in Vulkan will build a descriptor set that is passed to the shader
	
	struct VulkanMaterialData {
		VkDescriptorSetLayout descriptorLayout;
		VkDescriptorSet		  descriptorSet;
	};
	class VulkanMaterial : public Renderer::Material {
		Renderer::RenderDevice* _pdevice;
		VulkanMaterialData		_data;
		Renderer::Material::MaterialType _type;
		Buffer _buffer;
		std::vector<UniformBufferInfo> _bufferInfo;
		void BuildDiffuseColor(VulkContext&context);
		void BuildDiffuseTexture(VulkContext&context);
		void BuildDiffuseColorArray(VulkContext&context);
		void BuildDiffuseTextureArray(VulkContext&context);
	public:
		VulkanMaterial(Renderer::RenderDevice* pdevice,Renderer::Material::MaterialType type);
		virtual ~VulkanMaterial();
		
		virtual void SetAttributes(void* ptr, uint32_t size)override;		//pass a buffer of 1 or more elements (32-bit size) that holds triangle index
		virtual void SetData(void* ptr, uint32_t size)override;				//pass in a buffer of some structure holding material info, each shader will determine what the type is
		virtual void SetTextures(Renderer::Texture*ptr, uint32_t size)override;			//pass in an array (1 or more) textures, each shader will determine meaning.
		virtual void* GetMaterialData()override { return (void*)&_data; }	//pass the data to shader program
		virtual Renderer::Material::MaterialType GetMaterialType() override { return _type; }
	};
}
