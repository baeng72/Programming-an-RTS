#pragma once
#include <vector>
#include "../../Renderer/Shader.h"

#include "VulkanEx.h"
#include "VulkanShaderManager.h"
#include "../../Core/hash.h"
namespace Vulkan {
	//constexpr int MAX_IMAGE_COUNT = 8;
	//struct DescriptorBinding {
	//	union {
	//		VkBuffer buffer;		//handle of storage buffer or uniform buffer
	//		struct {
	//			
	//			VkImage images[MAX_IMAGE_COUNT];			//handle of image for texture
	//			uint32_t imageCount;
	//		};
	//	};
	//	
	//};

	//struct DescriptorSet {
	//	std::vector<DescriptorBinding> bindings;
	//};

	//struct Descriptors {
	//	std::vector<DescriptorSet> sets;
	//};

	
	class VulkanShader : public Renderer::Shader {
		Renderer::RenderDevice* _pdevice;
		VulkanShaderData* _pShaderData;
		std::vector<VkDescriptorSet>  _descriptorSets;
		//std::vector<Descriptors> _descriptors;	//holds the current values of the descriptor sets
		std::vector<std::vector<VkWriteDescriptorSet>> _writes;			//current values of images and buffers to be written
		std::vector<VkDescriptorImageInfo> _writeImages;	//space to hold all images to be written
		std::vector<VkDescriptorBufferInfo> _writeBuffers;	//space to hold all buffers to be written
		std::vector<DescriptorSetCache> _descriptorCache;	//get existing/new descriptor based on current value of writes
		//std::unordered_map<size_t, VkDescriptorSet> _textureDescriptors;
		//std::vector<std::vector<VkDescriptorBufferInfo>> bufferInfos;
		//std::vector<std::vector<std::vector<VkDescriptorImageInfo>>> imageInfos;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> texturemembers;
		std::unordered_map<size_t, int> texturehashmap;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> storagemembers;
		std::unordered_map<size_t, int> storagehashmap;
		//std::unordered_map<int32_t,size_t> storagehashes;
		//bool needsRebind;
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

		//virtual void Rebind(uint32_t* pdynoffsets = nullptr, uint32_t dynoffcount = 0)override;
		
	};
}
