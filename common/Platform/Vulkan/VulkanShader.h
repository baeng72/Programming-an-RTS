#pragma once
#include <vector>
#include "../../Renderer/Shader.h"

#include "VulkanEx.h"
#include "VulkanShaderManager.h"
#include "../../Core/hash.h"
namespace Vulkan {
	
	class VulkanShader : public Renderer::Shader {
		Renderer::RenderDevice* _pdevice;
		VulkanShaderData* _pShaderData;
		std::vector<VkDescriptorSet>  _descriptorSets;
		std::unordered_map<size_t, VkDescriptorSet> _textureDescriptors;
		std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;
		std::vector<std::vector<VkDescriptorImageInfo>> imageInfos;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> texturemembers;
		std::unordered_map<size_t, int> texturehashmap;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> storagemembers;
		std::unordered_map<size_t, int> storagehashmap;
	public:
		VulkanShader(Renderer::RenderDevice* pdevice,void*shaderData);		
		virtual ~VulkanShader();
		
		void Bind(uint32_t* pdynoffsets = nullptr, uint32_t dynoffcount = 0)override;
		
		//virtual void SetPushConstData(void*, uint32_t len)override;
		virtual void SetWireframe(bool wireframe)override;
		virtual bool SetUniformBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic = false) override;
		virtual bool SetUniformBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic = false) override;
		virtual bool SetUniformData(uint32_t i, void* ptr, uint32_t len, bool dynamic = false) override;
		virtual bool SetUniformData(const char* pname, void* ptr, uint32_t len, bool dynamic = false) override;
		virtual bool SetUniform(uint32_t i, vec2& v)override;
		virtual bool SetUniform(uint32_t i, vec2* p)override;
		virtual bool SetUniform(const char* pname, vec2& v)override;
		virtual bool SetUniform(const char* pname, vec2* p)override;
		virtual bool SetUniform(uint32_t i, vec3& v)override;
		virtual bool SetUniform(uint32_t i, vec3* p)override;
		virtual bool SetUniform(const char* pname, vec3& v)override;
		virtual bool SetUniform(const char* pname, vec3* p)override;
		virtual bool SetUniform(uint32_t i, vec4& v)override;
		virtual bool SetUniform(uint32_t i, vec4* p)override;
		virtual bool SetUniform(const char* pname, vec4& v)override;
		virtual bool SetUniform(const char* pname, vec4* p)override;
		virtual bool SetUniform(uint32_t i, mat4& v)override;
		virtual bool SetUniform(uint32_t i, mat4* p)override;
		virtual bool SetUniform(const char* pname, mat4& v)override;
		virtual bool SetUniform(const char* pname, mat4* p)override;
		virtual bool SetUniform(const char* pname, void* ptr, uint32_t len)override;
		virtual bool SetUniform(uint32_t i, void* ptr, uint32_t size) override;
		virtual uint32_t GetUniformId(const char* pname, bool dynamic = false) override;
		
		virtual bool SetTexture(uint32_t, Renderer::Texture** pptexture, uint32_t count) override;
		virtual bool SetTexture(const char* pname, Renderer::Texture** pptexture, uint32_t count) override;
		virtual bool SetTextures(Renderer::Texture** pptextures, uint32_t count) override;
		virtual uint32_t GetTextureId(const char* pname) override;
		virtual bool SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer, bool dynamic = false) override;
		virtual bool SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer, bool dynamic = false) override;
		virtual bool SetStorageData(uint32_t i, void* ptr, uint32_t len, bool dynamic = false) override;
		virtual bool SetStorageData(const char* pname, void* ptr, uint32_t len, bool dynamic = false) override;
		
		virtual uint32_t GetStorageId(const char* pname, bool dynamic = false) override;
		
	};
}
