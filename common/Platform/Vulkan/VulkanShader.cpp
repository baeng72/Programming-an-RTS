
#include <glm/glm.hpp>
#include "VulkanShader.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "VulkanBuffer.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"
#include <cstring>

namespace Vulkan {

	VulkanShader::VulkanShader(Renderer::RenderDevice* pdevice, void* shaderData) {
		_pShaderData = (VulkanShaderData*)shaderData;
		_pdevice = pdevice;
		//needsRebind = false;
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
			_descriptorCache.push_back({ layouts[set],context.pPoolCache,context.pLayoutCache });
		}
		//update descriptor sets, use useful defaults if not available
		VulkDefRes* defRes = reinterpret_cast<VulkDefRes*>(_pdevice->GetDefaultResources());
		
		_writes.resize(layouts.size() );
		_writeBuffers.reserve(16);
		_writeImages.reserve(16);
		uint32_t bufferCount = 0;
		uint32_t imageCount = 0;
		
		
		uint32_t offset=0;
		for (size_t s = 0; s < _pShaderData->reflection.bindings.size(); s++) {			
			auto& bindingset = _pShaderData->reflection.bindings[s];
			//resize each set of writes
			_writes[s].resize(bindingset.size());
			for (size_t b = 0; b < bindingset.size(); b++) {
				auto& binding = bindingset[b];
				uint32_t set = binding.set;
				uint32_t bindx = binding.binding;
				uint32_t count = binding.count;
				_writes[set][b].descriptorType = binding.descriptorType;
				_writes[set][b].descriptorCount = count;
				_writes[set][b].dstBinding = bindx;
				_writes[set][b].sType= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				switch (binding.descriptorType) {
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					
					//TODO: using pre-alloctated uniform, this may need to change to use externally allocated uniform
					_writeBuffers.push_back({ _pShaderData->uniformBuffer.buffer,offset,binding.getPaddedSize() });
					_writes[set][b].pBufferInfo = &_writeBuffers[bufferCount++];
					offset += binding.getPaddedSize();
					break;
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					_writeBuffers.push_back({ defRes->defUniformDynamic.buffer,0,defRes->defUniformDynamic.size });
					_writes[set][b].pBufferInfo = &_writeBuffers[bufferCount++];
					break;
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					_writeBuffers.push_back({ defRes->defStorage.buffer,0,defRes->defStorage.size });
					_writes[set][b].pBufferInfo = &_writeBuffers[bufferCount++];
					break;
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					_writeBuffers.push_back({ defRes->defStorageDynamic.buffer,0,defRes->defStorageDynamic.size });
					_writes[set][b].pBufferInfo = &_writeBuffers[bufferCount++];


					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
				{
					int firstImage = imageCount;

					for (uint32_t i = 0; i < count; i++,imageCount++) {
						_writeImages.push_back({ defRes->defTexture.sampler,defRes->defTexture.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });
					}
					_writes[set][b].pImageInfo = &_writeImages[firstImage];
					break;
				}
				}
			}
		}

		

		//textures are a uniform type, but treating differently, so copy to different datamap
		for (size_t m = 0; m < _pShaderData->reflection.blockmembers.size(); m++) {
			auto& member = _pShaderData->reflection.blockmembers[m];
			int parent = std::get<1>(member);
			
			uint32_t paddedSize = std::get<6>(member);
			if (parent == -1 && paddedSize == 0) {//a 'root' member, so uniform or buffer if parent==-1 and size of 0 indicates not a buffer type
				texturemembers.push_back(member);
			}
			else {
				int set = std::get<2>(member);
				int binding = std::get<3>(member);
				if (set >= 0) {
					switch (_pShaderData->reflection.bindings[set][binding].descriptorType) {
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
						storagemembers.push_back(member);
						break;

					}
				}
				
			}
		}
		for (size_t m = 0; m < texturemembers.size(); m++) {
			std::string& name = std::get<0>(texturemembers[m]);
			size_t hash = Core::HashFNV1A(name.c_str(), name.length());
			texturehashmap[hash] = (int)m;
		}

		for (size_t m = 0; m < storagemembers.size(); m++) {
			std::string& name = std::get<0>(storagemembers[m]);
			size_t hash = Core::HashFNV1A(name.c_str(), name.length());
			storagehashmap[hash] = (int)m;
		}
	
		

		
	}
	Vulkan::VulkanShader::~VulkanShader()
	{
		
		
	}
	
	void VulkanShader::Bind(uint32_t* pdynoffsets, uint32_t dynoffcount,bool override)
	{

		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		
		for (size_t i = 0; i < _writes.size(); i++) {
			if (override)
				_descriptorCache[i].Reset();
			_descriptorSets[i] = _descriptorCache[i].getDescriptor(_writes[i]);
		}
		vkCmdBindDescriptorSets(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipelineLayout, 0, (uint32_t)_descriptorSets.size(), _descriptorSets.data(), dynoffcount, pdynoffsets);
		vkCmdBindPipeline(framedata.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pShaderData->pipeline);
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
		VulkDefRes* defRes = reinterpret_cast<VulkDefRes*>(_pdevice->GetDefaultResources());
		//get binding for this index
		if (i >= 0) {
			//update descriptor set
			auto& member = _pShaderData->reflection.blockmembers[i];
			
			uint32_t set = std::get<2>(member);
			uint32_t binding = std::get<3>(member);
			//update all members who share this set/binding combo, as they'll be members of the same uniform block
			for (size_t j = i; j < _pShaderData->reflection.blockmembers.size(); j++) {
				int32_t s1 = std::get<2>(_pShaderData->reflection.blockmembers[j]);
				int32_t b1 = std::get<3>(_pShaderData->reflection.blockmembers[j]);
				uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[j]);
				if (set == s1 && binding == b1) {
					std::get<7>(_pShaderData->reflection.blockmembers[j]) = (void*)((uint8_t*)pdata->ptr + offset);//patch offset into pointers
				}
			}
			
			

			auto& bindingset = _pShaderData->reflection.bindings[set];
			auto& write = _writes[set][binding];
			switch (bindingset[binding].descriptorType) {
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: 
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				((VkDescriptorBufferInfo*)write.pBufferInfo)->buffer = pdata->buffer.buffer;
				((VkDescriptorBufferInfo*)write.pBufferInfo)->offset = 0;
				((VkDescriptorBufferInfo*)write.pBufferInfo)->range = pdata->size;			
				break;
			default:
				assert(0);
				break;
			}
			//auto& bufferInfo = bufferInfos[set];
			//auto& imageInfo = imageInfos[set];
			//
			////DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set]);
			//for (size_t b = 0; b < bindingset.size(); b++) {
			//	uint32_t count = bindingset[b].count;
			//	if (b == binding) {
			//		if (bufferInfo[b].buffer != pdata->buffer.buffer) {
			//			bufferInfo[b].buffer = pdata->buffer.buffer;
			//			bufferInfo[b].offset = 0;
			//			bufferInfo[b].range = pdata->size;
			//			needsRebind = true;
			//		}
			//	}
			//	switch (bindingset[b].descriptorType) {
			//	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			//	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			//	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			//	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			//		//updater.AddBinding((uint32_t)b, bindingset[b].descriptorType, &bufferInfo[b]);
			//		break;
			//	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			//		
			//		
			//		//updater.AddBinding((uint32_t)b, bindingset[b].descriptorType, imageInfo[b].data(),count);
			//		break;
			//	default:
			//		assert(0);
			//		break;
			//	}
			//}
			//updater.update();
			
			return true;
		}
		////need to update descriptor
		//if (i >= 0) {
		//	uint32_t setBinding = dynamic ? _pShaderData->uniformDynamicSetBindings[i] : _pShaderData->uniformSetBindings[i];
		//	uint32_t set = setBinding >> 16;
		//	uint32_t binding = setBinding & 0x00FF;
		//	VkDescriptorBufferInfo bufferInfo{};
		//	bufferInfo.buffer = pdata->buffer.buffer;
		//	bufferInfo.offset = 0;	//may not be true ?
		//	bufferInfo.range = pdata->size;
		//	DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set])
		//		.AddBinding(binding, pdata->descriptorType, &bufferInfo)
		//		.update();
		//	if (dynamic) {
		//		_pShaderData->uniformDynamicMap[_pShaderData->uniformDynamicNames[i]] = pdata->ptr;
		//		_pShaderData->uniformDynamicSizeMap[_pShaderData->uniformDynamicNames[i]] = pdata->size;
		//	}
		//	else {
		//		_pShaderData->uniformMap[_pShaderData->uniformNames[i]] = pdata->ptr;//use an array and index using id?
		//		_pShaderData->uboSizeMap[_pShaderData->uniformNames[i]] = pdata->size;
		//	}
		//	return true;
		//}
		return false;
	}

	bool VulkanShader::SetUniformBuffer(const char* pname, Renderer::Buffer* pbuffer,bool dynamic)
	{
		uint32_t id = GetUniformId(pname,dynamic);
		return SetUniformBuffer(id, pbuffer,dynamic);
	}

	bool VulkanShader::SetUniformData(uint32_t i, void*ptr,uint32_t len,bool dynamic)
	{
		auto& blockmembers = _pShaderData->reflection.blockmembers;
		assert(i>=0 && i < blockmembers.size());
		if (i < blockmembers.size()) {

			void* pdst = std::get<7>(blockmembers[i]);
			uint32_t size = std::get<6>(blockmembers[i]);
			uint32_t offset = std::get<4>(blockmembers[i]);
			if (pdst == nullptr) {
				assert(size == len);
				//push constant
				Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
				Vulkan::VulkFrameData& framedata = *framedataptr;
				vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->reflection.pushBlock.stageFlags, offset, size, ptr);
			}
			else {				
				memcpy(pdst, ptr, std::min(len, size));
			}
			return true;
		}
		
		return false;
	}

	bool VulkanShader::SetUniformData(const char* pname, void* ptr, uint32_t len,bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int i = _pShaderData->reflection.blockmap[hash];
		
		return SetUniformData(i, ptr, len, dynamic);
		
	}

	bool VulkanShader::SetUniform(const char* pname, int32_t v)
	{
		return SetUniform(pname, &v, sizeof(int32_t));
	}


	bool VulkanShader::SetUniform(uint32_t i, int32_t v)
	{
		return SetUniform(i, &v, sizeof(int32_t));
	}

	bool VulkanShader::SetUniform(const char* pname, float f)
	{
		return SetUniform(pname, &f, sizeof(float));
	}


	bool VulkanShader::SetUniform(uint32_t i, float f)
	{
		return SetUniform(i, &f, sizeof(float));
	}

	bool VulkanShader::SetUniform(uint32_t i, vec2& v)
	{
		return SetUniform(i, &v, sizeof(vec2));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, vec2* p)
	{
		return SetUniform(i, p, sizeof(vec2));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec2& v)
	{
		return SetUniform(pname, &v, sizeof(vec2));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec2* p)
	{
		return SetUniform(pname, p, sizeof(vec2));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, vec3& v)
	{
		return SetUniform(i, &v, sizeof(vec3));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, vec3* p)
	{
		return SetUniform(i, p, sizeof(vec3));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec3& v)
	{
		return SetUniform(pname, &v, sizeof(vec3));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec3* p)
	{
		return SetUniform(pname, p, sizeof(vec4));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, vec4& v)
	{
		return SetUniform(i, &v, sizeof(vec4));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, vec4* p)
	{
		return SetUniform(i, p, sizeof(vec4));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec4& v)
	{
		return SetUniform(pname, &v, sizeof(vec4));
		
	}

	bool VulkanShader::SetUniform(const char* pname, vec4* p)
	{
		return SetUniform(pname, p, sizeof(vec4));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, mat4& v)
	{
		return SetUniform(i, &v, sizeof(mat4));
		
	}

	bool VulkanShader::SetUniform(uint32_t i, mat4* p)
	{
		return SetUniform(i, p, sizeof(mat4));
		
	}

	bool VulkanShader::SetUniform(const char* pname, mat4& v)
	{
		return SetUniform(pname, &v, sizeof(mat4));
		
	}

	bool VulkanShader::SetUniform(const char* pname, mat4* p)
	{
		return SetUniform(pname, p, sizeof(mat4));
		
	}

	bool VulkanShader::SetUniform(const char* pname, void* ptr, uint32_t len) {
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		uint32_t i = _pShaderData->reflection.blockmap[hash];
		auto& blockmembers = _pShaderData->reflection.blockmembers;
		assert(i >= 0 && i < blockmembers.size());
		if (i < blockmembers.size()) {
			void* pdst = std::get<7>(_pShaderData->reflection.blockmembers[i]);
			uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
			if (pdst) {
				memcpy(pdst, ptr, len);
			}
			else {
				uint32_t size = std::get<6>(blockmembers[i]);
				Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
				Vulkan::VulkFrameData& framedata = *framedataptr;
				vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->reflection.pushBlock.stageFlags, offset, size, ptr);
			}
			return true;
		}		
		return false;
	}

	bool VulkanShader::SetUniform(uint32_t i, void* ptr, uint32_t size)
	{
		auto& blockmembers = _pShaderData->reflection.blockmembers;
		assert(i >= 0 && i < blockmembers.size());
		if (i < blockmembers.size()) {
			void* pdst = std::get<7>(_pShaderData->reflection.blockmembers[i]);
			uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
			if (pdst) {
				memcpy(pdst, ptr, size);
			}
			else {
				Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
				Vulkan::VulkFrameData& framedata = *framedataptr;
				vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->reflection.pushBlock.stageFlags, offset, size, ptr);
			}
			return true;
		}
		return false;
	}

	
	uint32_t VulkanShader::GetUniformId(const char* pname,bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		assert(_pShaderData->reflection.blockmap.find(hash) != _pShaderData->reflection.blockmap.end());
		return _pShaderData->reflection.blockmap[hash];
		
	}
	bool VulkanShader::SetTexture(uint32_t id, Renderer::Texture* ptexture) {
		assert(id < texturemembers.size());
		auto& member = texturemembers[id];
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;


		int set = std::get<2>(member);
		int bindx = std::get<3>(member);
		int writeidx = (int)reinterpret_cast<size_t>(std::get<7>(member));//use index into binding set above
		int imagecount = std::get<4>(member);//need to have 'offset' be count, using size as indicator that it's a texture
		auto& write = _writes[set][writeidx];
		assert(write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		assert(write.descriptorCount == imagecount);
		Vulkan::Texture* pvtext = (Vulkan::Texture*)ptexture->GetNativeHandle();
		VkDescriptorImageInfo* pinfo = (VkDescriptorImageInfo*)&write.pImageInfo[0];
		pinfo->imageView = pvtext->imageView;
		pinfo->sampler = pvtext->sampler;
		pinfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		return true;
	}

	bool VulkanShader::SetTexture(const char* pname, Renderer::Texture* ptexture) {
		uint32_t id = GetTextureId(pname);
		return SetTexture(id, ptexture);
	}
	
	bool VulkanShader::SetTexture(uint32_t id, Renderer::Texture** pptexture, uint32_t count)
	{
		assert(id < texturemembers.size());
		auto& member = texturemembers[id];
		
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		

		int set = std::get<2>(member);
		int bindx = std::get<3>(member);
		int writeidx = (int)reinterpret_cast<size_t>(std::get<7>(member));//use index into binding set above
		int imagecount = std::get<4>(member);//need to have 'offset' be count, using size as indicator that it's a texture
		auto& write = _writes[set][writeidx];
		assert(write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		assert(write.descriptorCount == imagecount);
		for (int i = 0; i < imagecount; i++) {
			Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();
			VkDescriptorImageInfo* pinfo = (VkDescriptorImageInfo*)&write.pImageInfo[i];
			pinfo->imageView = pvtext->imageView;
			pinfo->sampler = pvtext->sampler;
			pinfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		return true;
		//if (descriptor == VK_NULL_HANDLE) {
		//	
		//	DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
		//	auto& bindingset = _pShaderData->reflection.bindings[set];
		//	for (size_t b = 0; b < bindingset.size(); b++) {
		//		auto& binding = bindingset[b];
		//		builder.AddBinding((uint32_t)b, binding.descriptorType, binding.stageFlags, binding.count);
		//	}
		//	builder.build(descriptor);
		//	_textureDescriptors[hash] = descriptor;//store for future use
		//	assert(imagecount == count);
		//	//std::vector<VkDescriptorImageInfo> images(count);//need scratch for this, could become a problem if we have multiple texture arrays as we are not keeping the multiples and all that for other textures
		//	DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], descriptor);
		//	for (size_t b = 0; b < bindingset.size(); b++) {
		//		auto& binding = bindingset[b];
		//		auto& imageInfo = imageInfos[set][b];

		//		if (b == bindx) {
		//			
		//			
		//			for (uint32_t i = 0; i < count; i++) {
		//				Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();
		//				
		//				imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//				imageInfo[i].imageView = pvtext->imageView;
		//				imageInfo[i].sampler = pvtext->sampler;
		//			}
		//			updater.AddBinding((uint32_t)b, binding.descriptorType, imageInfo.data(),count);
		//		}
		//		else {
		//			//copy what we've saved
		//			switch (binding.descriptorType) {
		//			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		//			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		//			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		//			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		//				updater.AddBinding((uint32_t)b, binding.descriptorType, &bufferInfos[set][b]);
		//				break;
		//			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		//				//could be a bug, what if this is also a texture array?
		//			
		//				updater.AddBinding((uint32_t)b, binding.descriptorType, imageInfos[set][b].data(),(uint32_t)imageInfos[set][b].size());
		//				break;
		//			default:
		//				assert(0);
		//				break;
		//			}

		//		}
		//	}
		//	updater.update();

		//}
		//if (descriptor != VK_NULL_HANDLE && _descriptorSets[set]!=descriptor) {
		//	_descriptorSets[set] = descriptor;//use descriptor on next bind
		//	needsRebind = true;
		//	return true;
		//}
		//return false;

		
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
		
		uint32_t imageIndex = 0;
		//for (size_t s = 0; s < _pShaderData->reflection.bindings.size(); s++) {

			//std::vector< std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> cursettuple;
			for (size_t m = 0; m < texturemembers.size(); m++) {
				auto& member = texturemembers[m];
				int set = std::get<2>(member);
				int binding = std::get<3>(member);
				int writeidx =(int)reinterpret_cast<size_t>(std::get<7>(member));//index set above
				/*if (set == (int)s) {
					cursettuple.push_back(member);
				}*/
				auto& write = _writes[set][writeidx];
				Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[imageIndex++]->GetNativeHandle();
				((VkDescriptorImageInfo*)write.pImageInfo)->imageView = ptextdata->imageView;
				((VkDescriptorImageInfo*)write.pImageInfo)->sampler = ptextdata->sampler;
				((VkDescriptorImageInfo*)write.pImageInfo)->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}
			//if (cursettuple.size()) {
			//	
			//	uint32_t index = 0;
			//	size_t hash = 0;
			//	for (size_t i = 0; i < cursettuple.size(); i++) {
			//		Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
			//		hash ^= HASH(ptextdata);					
			//	}
			//	VkDescriptorSet descriptor = _textureDescriptors[hash];
			//	if (descriptor == VK_NULL_HANDLE) {
			//		auto setbuilder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
			//		for (size_t b = 0; b < cursettuple.size(); b++) {

			//			setbuilder.AddBinding(std::get<3>(cursettuple[b]), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
			//		}
			//		setbuilder.build(descriptor);
			//		_textureDescriptors[hash] = descriptor;

			//		auto setupdater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[s], descriptor);
			//		index = 0;
			//		//std::vector<VkDescriptorImageInfo> imageInfos(cursettuple.size());
			//		for (size_t b = 0; b < cursettuple.size(); b++) {
			//			uint32_t count = std::get<4>(cursettuple[b]);
			//			for (size_t c = 0; c < count; c++) {
			//				Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
			//				imageInfos[s][b][c].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			//				imageInfos[s][b][c].imageView = ptextdata->imageView;
			//				imageInfos[s][b][c].sampler = ptextdata->sampler;
			//				
			//			}
			//			uint32_t bindx = std::get<3>(cursettuple[b]);
			//			setupdater.AddBinding(bindx, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageInfos[s][b].data(),count);
			//		}
			//		setupdater.update();
			//	}
			//	if (descriptor != VK_NULL_HANDLE&& _descriptorSets[s]!=descriptor) {
			//		_descriptorSets[s] = descriptor;
			//		needsRebind = true;
			//		return true;
			//	}
			//}
		//}
		return true;
		
	}

	
	uint32_t VulkanShader::GetTextureId(const char* pname)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		ASSERT(texturehashmap.find(hash) != texturehashmap.end(), "Unkown image name!");
		return texturehashmap[hash];
		/*auto& names = _pShaderData->imageNames;
		ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown image name!");
		return (uint32_t)std::distance(names.begin(), std::find(names.begin(), names.end(), pname));*/
	}

	bool VulkanShader::SetStorageBuffer(uint32_t i, Renderer::Buffer* pbuffer,bool dynamic)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VulkanBufferData* pdata = (VulkanBufferData*)pbuffer->GetNativeHandle();
		ASSERT(i>=0 && i < storagemembers.size(), "Unknown storage slot!");

		size_t hash = HASH(pdata);
		auto& member = storagemembers[i];
		int s = std::get<2>(member);
		int b = std::get<3>(member);
		auto& write = _writes[s][b];
		((VkDescriptorBufferInfo*)write.pBufferInfo)->buffer = pdata->buffer.buffer;
		((VkDescriptorBufferInfo*)write.pBufferInfo)->range = pdata->size;
		
		//int32_t bindmask = s << 16 | b;//this specific slot
		//if(storagehashes[bindmask]!=hash){//only update when need to change buffer
		//	uint32_t index = 0;
		//	for (size_t m = 0; m < storagemembers.size(); m++) {
		//		auto& member = storagemembers[m];
		//		int parent = std::get<1>(member);
		//		int set = std::get<2>(member);
		//		int bindx = std::get<3>(member);
		//		int offset = std::get<4>(member);
		//		if (s == set && b == bindx) {
		//			std::get<7>(member) = (void*)((uint8_t*)pdata->ptr + offset);
		//			if (parent == -1) {
		//				if (index == i) {
		//					auto& buffers = bufferInfos[set];
		//					auto& bufferInfo = buffers[bindx];
		//					bufferInfo.buffer = pdata->buffer.buffer;
		//					bufferInfo.offset = 0;	//may not be true ?
		//					bufferInfo.range = pdata->size;
		//					auto& bindingset = _pShaderData->reflection.bindings[set];
		//					DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set]);
		//					for (size_t b = 0; b < bindingset.size(); b++) {
		//						auto& binding = bindingset[b];
		//						auto count = bindingset[b].count;
		//						switch (binding.descriptorType) {
		//						case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		//						case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		//						case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		//						case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		//							updater.AddBinding(binding.binding, binding.descriptorType, &buffers[bindx]);
		//							break;
		//						case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		//							updater.AddBinding(binding.binding, binding.descriptorType, imageInfos[set][bindx].data(),count);
		//						}

		//					}
		//					updater.update();


		//				}

		//				index++;
		//			}
		//		}

		//	}
		//	storagehashes[bindmask] = hash;
		//}
		return true;
		



	
	}

	bool VulkanShader::SetStorageBuffer(const char* pname, Renderer::Buffer* pbuffer,bool dynamic)
	{
		//need to update descriptor
		uint32_t id = GetStorageId(pname,dynamic);
		return SetStorageBuffer(id, pbuffer,dynamic);		
	}

	bool VulkanShader::SetStorageData(uint32_t i, void* ptr, uint32_t len,bool dynamic)
	{
		ASSERT(i >= 0 && i < storagemembers.size(), "Unknown storage slot!");
		auto& member = storagemembers[i];
		memcpy(std::get<7>(member), ptr, len);
		return true;
		/*if (dynamic) {
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
		return false;*/
	}

	bool VulkanShader::SetStorageData(const char* pname, void* ptr, uint32_t len,bool dynamic)
	{
		uint32_t i = GetStorageId(pname,dynamic);
		ASSERT(i >= 0 && i < storagemembers.size(), "Unknown storage slot!");
		auto& member = storagemembers[i];
		memcpy(std::get<7>(member), ptr, len);
		return true;
		
		
	}

	

	uint32_t VulkanShader::GetStorageId(const char* pname,bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		ASSERT(storagehashmap.find(hash)!=storagehashmap.end(), "Unknown storage slot!");
		return storagehashmap[hash];
		
	}

	void* VulkanShader::GetNativeHandle()
	{
		return &_pShaderData->pipeline;
	}

	

	/*void VulkanShader::Rebind(uint32_t* pdynoffsets, uint32_t dynoffcount)
	{
		if (needsRebind) {
			Bind(pdynoffsets, dynoffcount);
			needsRebind = false;
		}
	}*/
	
}
