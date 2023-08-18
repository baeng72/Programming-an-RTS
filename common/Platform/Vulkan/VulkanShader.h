#pragma once
#include <vector>
#include "../../Renderer/Shader.h"

#include "VulkanEx.h"
#include "VulkanShaderManager.h"

namespace Vulkan {
	
	class VulkanShader : public Renderer::Shader {
		Renderer::RenderDevice* _pdevice;
		VulkanShaderData* _pShaderData;
		std::vector<VkDescriptorSet>  _descriptorSets;
		
	public:
		VulkanShader(Renderer::RenderDevice* pdevice,void*shaderData);		
		virtual ~VulkanShader();
		
		void Bind()override;
		
		virtual void SetPushConstData(void*, uint32_t len)override;
		virtual void SetWireframe(bool wireframe)override;
		virtual bool SetUniformData(uint32_t i, void* ptr, uint32_t len) override;
		virtual bool SetUniformData(const char* pname, void* ptr, uint32_t len) override;
		
		virtual uint32_t GetUniformId(const char* pname) override;
		virtual bool SetTexture(uint32_t, Renderer::Texture** pptexture, uint32_t count) override;
		virtual bool SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count) override;
		
		virtual uint32_t GetTextureId(const char* pname) override;
		virtual bool SetStorage(uint32_t i, Renderer::Buffer* pbuffer) override;
		virtual bool SetStorage(const char* pname, Renderer::Buffer* pbuffer) override;
		virtual bool SetStorageData(uint32_t i, void* ptr, uint32_t len) override;
		virtual bool SetStorageData(const char* pname, void* ptr, uint32_t len) override;
		
		virtual uint32_t GetStorageId(const char* pname) override;
	};
}
