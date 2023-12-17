
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
		needsRebind = false;
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
		//update descriptor sets, use useful defaults if not available
		VulkDefRes *defRes = reinterpret_cast<VulkDefRes*>(_pdevice->GetDefaultResources());
		uint32_t offset=0;
		bufferInfos.resize(_pShaderData->reflection.bindings.size());
		imageInfos.resize(_pShaderData->reflection.bindings.size());
		for (size_t s = 0; s < _pShaderData->reflection.bindings.size(); s++) {
			auto& bindingset = _pShaderData->reflection.bindings[s];
			std::vector<VkDescriptorBufferInfo>& buffers = bufferInfos[s];
			buffers.resize(bindingset.size());
			std::vector<VkDescriptorImageInfo>& images = imageInfos[s];
			images.resize(bindingset.size());
			auto updater = DescriptorSetUpdater::begin(context.pLayoutCache, layouts[s], sets[s]);
			for (size_t b = 0; b < bindingset.size(); b++) {
				auto& binding = bindingset[b];
				uint32_t set = binding.set;
				uint32_t bindx = binding.binding;
				switch (binding.descriptorType) {
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					buffers[bindx].buffer = _pShaderData->uniformBuffer.buffer;
					buffers[bindx].offset = offset;
					buffers[bindx].range = binding.getPaddedSize();
					offset += binding.getPaddedSize();
					updater.AddBinding(bindx, binding.descriptorType, &buffers[bindx]);
					break;
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					buffers[bindx].buffer = defRes->defUniformDynamic.buffer;
					buffers[bindx].offset = 0;
					buffers[bindx].range = defRes->defUniformDynamicInfo[0].objectSize;
					updater.AddBinding(bindx, binding.descriptorType, &buffers[bindx]);
					break;
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					buffers[bindx].buffer = defRes->defStorage.buffer;
					buffers[bindx].offset = 0;
					buffers[bindx].range = defRes->defStorageInfo[0].objectSize;
					updater.AddBinding(bindx, binding.descriptorType, &buffers[bindx]);
					break;
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					buffers[bindx].buffer = defRes->defStorageDynamic.buffer;
					buffers[bindx].offset = 0;
					buffers[bindx].range = defRes->defStorageDynamicInfo[0].objectSize;
					updater.AddBinding(bindx, binding.descriptorType, &buffers[bindx]);
					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					images[bindx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					images[bindx].imageView = defRes->defTexture.imageView;
					images[bindx].sampler = defRes->defTexture.sampler;
					updater.AddBinding(bindx, binding.descriptorType, &images[bindx]);
					break;
				default:
					assert(0);
					break;
				}
			}
			updater.update();
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
		//uint32_t index = 0;
		//VkDeviceSize offset = 0;
		//auto& uboSetBindings = _pShaderData->uniformSetBindings;
		//auto& uniformBuffer = _pShaderData->uniformBuffer;
		////need a descriptor set
		//if (uboSetBindings.size() > 0) {
		//	uint32_t set = uboSetBindings[0] >> 16;
		//	std::vector<VkDescriptorBufferInfo> bufferInfos(16);
		//	auto uniupdated = DescriptorSetUpdater::begin(context.pLayoutCache, layouts[set], sets[set]);
		//	for (auto& uboBinding : uboSetBindings) {
		//		uint32_t set = uboBinding >> 16;
		//		uint32_t binding = uboBinding & 0xFF;
		//		
		//		
		//		bufferInfos[binding].buffer = uniformBuffer.buffer;
		//		bufferInfos[binding].offset = offset;
		//		
		//		auto& name = _pShaderData->uniformNames[index];
		//		VkDeviceSize size = _pShaderData->uboSizeMap[name];
		//		bufferInfos[binding].range = size;
		//		index++;
		//		offset += size;

		//		uniupdated.AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfos[binding]);

		//		
		//	}
		//	uniupdated.update();
		//	
		//}
		

		
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

	/*void VulkanShader::SetPushConstData(void* pdata, uint32_t size) {
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;

		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, 0, size, pdata);

	}*/

	void VulkanShader::SetWireframe(bool wireframe)
	{
		_pShaderData->pipeline = wireframe ? _pShaderData->wireframePipeline : _pShaderData->filledPipeline;
	}

	bool VulkanShader::SetUniformBuffer(uint32_t i, Renderer::Buffer* pbuffer,bool dynamic)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VulkanBufferData* pdata = (VulkanBufferData*)pbuffer->GetNativeHandle();
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
				if (set = s1 && binding == b1) {
					std::get<7>(_pShaderData->reflection.blockmembers[j]) = (void*)((uint8_t*)pdata->ptr + offset);//patch offset into pointers
				}
			}


			auto& bindingset = _pShaderData->reflection.bindings[set];
			auto& bufferInfo = bufferInfos[set];
			auto& imageInfo = imageInfos[set];
			DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set]);
			for (size_t b = 0; b < bindingset.size(); b++) {
				if (b == binding) {
					bufferInfo[b].buffer = pdata->buffer.buffer;
					bufferInfo[b].offset = 0;
					bufferInfo[b].range = pdata->size;
				}
				switch (bindingset[b].descriptorType) {
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
					updater.AddBinding((uint32_t)b, bindingset[b].descriptorType, &bufferInfo[b]);
					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					updater.AddBinding((uint32_t)b, bindingset[b].descriptorType, &imageInfo[b]);
					break;
				default:
					assert(0);
					break;
				}
			}
			updater.update();
			
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
		/*if (dynamic) {
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
			assert(i < _pShaderData->uniformNames.size());
			if (i < _pShaderData->uniformNames.size()) {
				auto& name = _pShaderData->uniformNames[i];
				VkDeviceSize size = _pShaderData->uboSizeMap[name];
				void* pdst = _pShaderData->uniformMap[name];
				memcpy(pdst, ptr, std::min(len, (uint32_t)size));
				return true;
			}
		}*/
		return false;
	}

	bool VulkanShader::SetUniformData(const char* pname, void* ptr, uint32_t len,bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int i = _pShaderData->reflection.blockmap[hash];
		
		return SetUniformData(i, ptr, len, dynamic);
		/*{
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
				auto& names = _pShaderData->uniformNames;
				auto& pcnames = _pShaderData->pushConstNames;
				if (std::find(names.begin(), names.end(), name) == names.end()) {
					if (std::find(pcnames.begin(), pcnames.end(), name) == pcnames.end()) {
						ASSERT(std::find(names.begin(), names.end(), name) != names.end(), "Unknown shader name!");
					}
					else {
						Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
						Vulkan::VulkFrameData& framedata = *framedataptr;
						auto range = _pShaderData->pushConstRanges[std::distance(pcnames.begin(), std::find(pcnames.begin(), pcnames.end(), name))];
						vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, 0, range.size, ptr);
					}
				}
				else {

					if (_pShaderData->uniformMap.find(name) != _pShaderData->uniformMap.end()) {
						VkDeviceSize size = _pShaderData->uboSizeMap[name];
						void* pdst = _pShaderData->uniformMap[name];
						memcpy(pdst, ptr, std::min(len, (uint32_t)size));
						return true;

					}
				}
			}
		}
		return false;*/
	}

	bool VulkanShader::SetUniform(uint32_t i, vec2& v)
	{
		return SetUniform(i, &v, sizeof(vec2));
		//auto& blockmembers = _pShaderData->reflection.blockmembers;
		//assert(i >= 0 && i < blockmembers.size());
		//if (i < blockmembers.size()) {
		//	void* ptr = std::get<7>(_pShaderData->reflection.blockmembers[i]);
		//	uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
		//	if (ptr) {
		//		memcpy(ptr, &v, sizeof(vec2));
		//	}
		//	else {
		//		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//		Vulkan::VulkFrameData& framedata = *framedataptr;
		//		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, offset, sizeof(vec2), ptr);
		//	}

		//	return true;
		//}
		///*size_t nameSize = _pShaderData->uniformCombinedNames.size();
		//if (i < nameSize) {
		//	auto& name = _pShaderData->uniformCombinedNames[i];
		//	memcpy(_pShaderData->uniformCombinedMap[name], &v, sizeof(vec2));

		//	return true;
		//}*/
		//return false;
	}

	bool VulkanShader::SetUniform(uint32_t i, vec2* p)
	{
		return SetUniform(i, p, sizeof(vec2));
		//auto& blockmembers = _pShaderData->reflection.blockmembers;
		//assert(i >= 0 && i < blockmembers.size());
		//if (i < blockmembers.size()) {
		//	void* ptr = std::get<7>(_pShaderData->reflection.blockmembers[i]);
		//	uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
		//	if (ptr) {
		//		memcpy(ptr, p, sizeof(vec2));
		//	}
		//	else {
		//		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//		Vulkan::VulkFrameData& framedata = *framedataptr;
		//		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, offset, sizeof(vec2), ptr);
		//	}
		//	return true;
		//}
		///*size_t nameSize = _pShaderData->uniformCombinedNames.size();
		//if (i < nameSize) {
		//	auto& name = _pShaderData->uniformCombinedNames[i];
		//	memcpy(_pShaderData->uniformCombinedMap[name], p, sizeof(vec2));
		//	return true;
		//}*/
		//return false;
	}

	bool VulkanShader::SetUniform(const char* pname, vec2& v)
	{
		return SetUniform(pname, &v, sizeof(vec2));
		//size_t hash = Core::HashFNV1A(pname, strlen(pname));
		//uint32_t i = _pShaderData->reflection.blockmap[hash];
		//auto& blockmembers = _pShaderData->reflection.blockmembers;
		//assert(i >= 0 && i < blockmembers.size());
		//if (i < blockmembers.size()) {
		//	void* ptr = std::get<7>(_pShaderData->reflection.blockmembers[i]);
		//	uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
		//	if (ptr) {
		//		memcpy(ptr, &v, sizeof(vec2));
		//	}
		//	else {
		//		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//		Vulkan::VulkFrameData& framedata = *framedataptr;
		//		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, offset, sizeof(vec2), ptr);
		//	}
		//	return true;
		//}
		///*auto& uniformNames = _pShaderData->uniformCombinedNames;
		//if (std::find(uniformNames.begin(), uniformNames.end(), pname) != uniformNames.end()) {
		//	memcpy(_pShaderData->uniformCombinedMap[pname], &v, sizeof(vec2));
		//	return true;
		//}
		//else {
		//	auto& pcnames = _pShaderData->pushConstNames;
		//	if (std::find(pcnames.begin(), pcnames.end(), pname) != pcnames.end()) {

		//	}
		//}*/
		//return false;
	}

	bool VulkanShader::SetUniform(const char* pname, vec2* p)
	{
		return SetUniform(pname, p, sizeof(vec2));
		//size_t hash = Core::HashFNV1A(pname, strlen(pname));
		//uint32_t i = _pShaderData->reflection.blockmap[hash];
		//auto& blockmembers = _pShaderData->reflection.blockmembers;
		//assert(i >= 0 && i < blockmembers.size());
		//if (i < blockmembers.size()) {
		//	void* ptr = std::get<7>(_pShaderData->reflection.blockmembers[i]);
		//	uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
		//	if (ptr) {
		//		memcpy(ptr, p, sizeof(vec2));
		//	}
		//	else {
		//		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//		Vulkan::VulkFrameData& framedata = *framedataptr;
		//		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, offset, sizeof(vec2), ptr);
		//	}
		//	return true;
		//}
		////memcpy(_pShaderData->uniformCombinedMap[pname], p, sizeof(vec2));
		//return false;
	}

	bool VulkanShader::SetUniform(uint32_t i, vec3& v)
	{
		return SetUniform(i, &v, sizeof(vec3));
		//auto& blockmembers = _pShaderData->reflection.blockmembers;
		//assert(i >= 0 && i < blockmembers.size());
		//if (i < blockmembers.size()) {
		//	void* ptr = std::get<7>(_pShaderData->reflection.blockmembers[i]);
		//	uint32_t offset = std::get<4>(_pShaderData->reflection.blockmembers[i]);
		//	if (ptr) {
		//		memcpy(ptr, &v, sizeof(vec3));
		//	}
		//	else {
		//		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//		Vulkan::VulkFrameData& framedata = *framedataptr;
		//		vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->pushConstStages, offset, sizeof(vec3), ptr);
		//	}

		//	return true;
		//}
		///*auto& name = _pShaderData->uniformCombinedNames[i];
		//memcpy(_pShaderData->uniformCombinedMap[name], &v, sizeof(vec3));*/
		//return false;
	}

	bool VulkanShader::SetUniform(uint32_t i, vec3* p)
	{
		return SetUniform(i, p, sizeof(vec3));
		/*auto& name = _pShaderData->uniformCombinedNames[i];
		memcpy(_pShaderData->uniformCombinedMap[name], p, sizeof(vec3));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, vec3& v)
	{
		return SetUniform(pname, &v, sizeof(vec3));
		/*memcpy(_pShaderData->uniformCombinedMap[pname], &v, sizeof(vec3));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, vec3* p)
	{
		return SetUniform(pname, p, sizeof(vec4));
		/*memcpy(_pShaderData->uniformCombinedMap[pname], p, sizeof(vec3));
		return true;*/
	}

	bool VulkanShader::SetUniform(uint32_t i, vec4& v)
	{
		return SetUniform(i, &v, sizeof(vec4));
		/*auto& name = _pShaderData->uniformCombinedNames[i];
		memcpy(_pShaderData->uniformCombinedMap[name], &v, sizeof(vec4));
		return true;*/
	}

	bool VulkanShader::SetUniform(uint32_t i, vec4* p)
	{
		return SetUniform(i, p, sizeof(vec4));
		/*auto& name = _pShaderData->uniformCombinedNames[i];
		memcpy(_pShaderData->uniformCombinedMap[name], p, sizeof(vec4));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, vec4& v)
	{
		return SetUniform(pname, &v, sizeof(vec4));
		/*memcpy(_pShaderData->uniformCombinedMap[pname], &v, sizeof(vec4));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, vec4* p)
	{
		return SetUniform(pname, p, sizeof(vec4));
		/*memcpy(_pShaderData->uniformCombinedMap[pname], p, sizeof(vec3));
		return true;*/
	}

	bool VulkanShader::SetUniform(uint32_t i, mat4& v)
	{
		return SetUniform(i, &v, sizeof(mat4));
		/*auto& name = _pShaderData->uniformCombinedNames[i];
		memcpy(_pShaderData->uniformCombinedMap[name], &v, sizeof(mat4));
		return true;*/
	}

	bool VulkanShader::SetUniform(uint32_t i, mat4* p)
	{
		return SetUniform(i, p, sizeof(mat4));
		/*auto& name = _pShaderData->uniformCombinedNames[i];
		memcpy(_pShaderData->uniformCombinedMap[name], p, sizeof(mat4));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, mat4& v)
	{
		return SetUniform(pname, &v, sizeof(mat4));
		/*memcpy(_pShaderData->uniformCombinedMap[pname], &v, sizeof(mat4));
		return true;*/
	}

	bool VulkanShader::SetUniform(const char* pname, mat4* p)
	{
		return SetUniform(pname, p, sizeof(mat4));
		//memcpy(_pShaderData->uniformCombinedMap[pname], p, sizeof(mat4));
		//return true;
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
				Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
				Vulkan::VulkFrameData& framedata = *framedataptr;
				vkCmdPushConstants(framedata.cmd, _pShaderData->pipelineLayout, _pShaderData->reflection.pushBlock.stageFlags, offset, len, ptr);
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
		return _pShaderData->reflection.blockmap[hash];
		/*auto& names = dynamic ? _pShaderData->uniformDynamicNames : _pShaderData->uniformNames;
		ASSERT(std::find(names.begin(), names.end(), pname) != names.end(), "Unkown uniform name!");		
		return (uint32_t)std::distance(names.begin(),std::find(names.begin(), names.end(), pname));*/
	}

	bool VulkanShader::SetTexture(uint32_t id, Renderer::Texture** pptexture, uint32_t count)
	{
		assert(id < texturemembers.size());
		auto& member = texturemembers[id];
		size_t hash = 0;
		for (size_t i = 0; i < count; i++) {
			Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();			
			hash ^= HASH(pvtext);
		}
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		VkDescriptorSet descriptor = _textureDescriptors[hash];

		int set = std::get<2>(member);
		int bindx = std::get<3>(member);
		int imagecount = std::get<4>(member);//need to have 'offset' be count, using size as indicator that it's a texture
		if (descriptor == VK_NULL_HANDLE) {
			
			DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
			auto& bindingset = _pShaderData->reflection.bindings[set];
			for (size_t b = 0; b < bindingset.size(); b++) {
				auto& binding = bindingset[b];
				builder.AddBinding((uint32_t)b, binding.descriptorType, binding.stageFlags, binding.count);
			}
			builder.build(descriptor);
			_textureDescriptors[hash] = descriptor;//store for future use
			assert(imagecount == count);
			std::vector<VkDescriptorImageInfo> images(count);//need scratch for this, could become a problem if we have multiple texture arrays as we are not keeping the multiples and all that for other textures
			DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], descriptor);
			for (size_t b = 0; b < bindingset.size(); b++) {
				auto& binding = bindingset[b];
				if (b == bindx) {
					for (uint32_t i = 0; i < count; i++) {
						Vulkan::Texture* pvtext = (Vulkan::Texture*)pptexture[i]->GetNativeHandle();
						images[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						images[i].imageView = pvtext->imageView;
						images[i].sampler = pvtext->sampler;
					}
					updater.AddBinding((uint32_t)b, binding.descriptorType, images.data(),count);
				}
				else {
					//copy what we've saved
					switch (binding.descriptorType) {
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
						updater.AddBinding((uint32_t)b, binding.descriptorType, &bufferInfos[set][b]);
						break;
					case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						//could be a bug, what if this is also a texture array?
						updater.AddBinding((uint32_t)b, binding.descriptorType, &imageInfos[set][b]);
						break;
					default:
						assert(0);
						break;
					}

				}
			}
			updater.update();

		}
		if (descriptor != VK_NULL_HANDLE && _descriptorSets[set]!=descriptor) {
			_descriptorSets[set] = descriptor;//use descriptor on next bind
			needsRebind = true;
			return true;
		}
		return false;

		/*uint32_t setBinding = _pShaderData->imageSetBindings[id];
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
		return false;*/
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
		
		
		for (size_t s = 0; s < _pShaderData->reflection.bindings.size(); s++) {

			std::vector< std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>> cursettuple;
			for (size_t m = 0; m < texturemembers.size(); m++) {
				auto& member = texturemembers[m];
				int set = std::get<2>(member);
				int binding = std::get<3>(member);
				if (set == (int)s) {
					cursettuple.push_back(member);
				}
			}
			if (cursettuple.size()) {
				uint32_t index = 0;
				size_t hash = 0;
				for (size_t i = 0; i < cursettuple.size(); i++) {
					Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
					hash ^= HASH(ptextdata);					
				}
				VkDescriptorSet descriptor = _textureDescriptors[hash];
				if (descriptor == VK_NULL_HANDLE) {
					auto setbuilder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
					for (size_t b = 0; b < cursettuple.size(); b++) {

						setbuilder.AddBinding(std::get<3>(cursettuple[b]), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
					}
					setbuilder.build(descriptor);
					_textureDescriptors[hash] = descriptor;

					auto setupdater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[s], descriptor);
					index = 0;
					//std::vector<VkDescriptorImageInfo> imageInfos(cursettuple.size());
					for (size_t b = 0; b < cursettuple.size(); b++) {

						Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
						imageInfos[s][b].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						imageInfos[s][b].imageView = ptextdata->imageView;
						imageInfos[s][b].sampler = ptextdata->sampler;
						uint32_t bindx = std::get<3>(cursettuple[b]);
						setupdater.AddBinding(bindx, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfos[s][b]);
					}
					setupdater.update();
				}
				if (descriptor != VK_NULL_HANDLE&& _descriptorSets[s]!=descriptor) {
					_descriptorSets[s] = descriptor;
					needsRebind = true;
					return true;
				}
			}
		}
		return true;
		//ASSERT(count == _pShaderData->imageNames.size(), "Invalid number of textures!");
		//
		//
		//	
		//if (count <= _pShaderData->imageNames.size()) {
		//	auto setBindings = _pShaderData->imageSetBindings;
		//	std::vector<std::vector<uint32_t>> groupedSetBindings(_pShaderData->descriptorSetLayouts.size());//in case they are all over the shop
		//	for (uint32_t i = 0; i < count; i++) {
		//		uint32_t setbinding = setBindings[i];
		//		uint32_t set = setbinding >> 16;
		//		uint32_t binding = setbinding & 0xFF;
		//		groupedSetBindings[set].push_back(binding);
		//	}
		//	uint32_t index = 0;
		//	for (size_t set = 0; set < groupedSetBindings.size(); set++) {
		//		auto& groupedBindings = groupedSetBindings[set];
		//		if (groupedBindings.size() == 0)
		//			continue;
		//		size_t hash = 0;
		//		for (size_t b = 0; b < groupedBindings.size(); b++) {
		//			Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
		//			hash ^= HASH(ptextdata);
		//		}
		//		index = 0;
		//		VkDescriptorSet descriptor = _textureDescriptors[hash];
		//		if (descriptor == VK_NULL_HANDLE) {
		//			auto setbuilder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
		//			for (size_t b = 0; b < groupedBindings.size(); b++) {

		//				setbuilder.AddBinding(groupedBindings[b], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		//			}
		//			setbuilder.build(descriptor);
		//			_textureDescriptors[hash] = descriptor;
		//			auto setupdater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], descriptor);

		//			std::vector<VkDescriptorImageInfo> imageInfos(groupedBindings.size());
		//			for (size_t b = 0; b < groupedBindings.size(); b++) {
		//			
		//				Vulkan::Texture* ptextdata = (Vulkan::Texture*)pptextures[index++]->GetNativeHandle();
		//				imageInfos[b].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//				imageInfos[b].imageView = ptextdata->imageView;
		//				imageInfos[b].sampler = ptextdata->sampler;
		//				setupdater.AddBinding(groupedBindings[b], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfos[b]);
		//			}
		//			setupdater.update();
		//			_textureDescriptors[hash] = descriptor;
		//		}
		//		if (descriptor != VK_NULL_HANDLE) {
		//			_descriptorSets[set] = descriptor;
		//			return true;
		//		}
		//	}
		//	return true;
		//}
		//
		//
		//return false;
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
		
		int32_t bindmask = s << 16 | b;//this specific slot
		if(storagehashes[bindmask]!=hash){//only update when need to change buffer
			uint32_t index = 0;
			for (size_t m = 0; m < storagemembers.size(); m++) {
				auto& member = storagemembers[m];
				int parent = std::get<1>(member);
				int set = std::get<2>(member);
				int bindx = std::get<3>(member);
				int offset = std::get<4>(member);
				if (s == set && b == bindx) {
					std::get<7>(member) = (void*)((uint8_t*)pdata->ptr + offset);
					if (parent == -1) {
						if (index == i) {
							auto& buffers = bufferInfos[set];
							auto& bufferInfo = buffers[bindx];
							bufferInfo.buffer = pdata->buffer.buffer;
							bufferInfo.offset = 0;	//may not be true ?
							bufferInfo.range = pdata->size;
							auto& bindingset = _pShaderData->reflection.bindings[set];
							DescriptorSetUpdater updater = DescriptorSetUpdater::begin(context.pLayoutCache, _pShaderData->descriptorSetLayouts[set], _descriptorSets[set]);
							for (size_t b = 0; b < bindingset.size(); b++) {
								auto& binding = bindingset[b];
								switch (binding.descriptorType) {
								case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
								case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
								case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
								case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
									updater.AddBinding(binding.binding, binding.descriptorType, &buffers[bindx]);
									break;
								case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
									updater.AddBinding(binding.binding, binding.descriptorType, &imageInfos[set][bindx]);
								}

							}
							updater.update();


						}

						index++;
					}
				}

			}
			storagehashes[bindmask] = hash;
		}
		return true;
		



		////need to update descriptor
		//if (i >= 0) {
		//	uint32_t setBinding = dynamic ? _pShaderData->storageSetDynamicBindings[i] : _pShaderData->storageSetBindings[i];
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
		//		_pShaderData->storageDynamicMap[_pShaderData->storageDynamicNames[i]] = pdata->ptr;
		//		_pShaderData->storageDynamicSizeMap[_pShaderData->storageDynamicNames[i]] = pdata->size;
		//	}
		//	else {
		//		 _pShaderData->storageMap[_pShaderData->storageNames[i]] = pdata->ptr;//use an array and index using id?
		//		_pShaderData->storageSizeMap[_pShaderData->storageNames[i]] = pdata->size;
		//	}
		//	return true;
		//}
		//return false;
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
		
		/*if (dynamic) {
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
		return false;*/
	}

	

	uint32_t VulkanShader::GetStorageId(const char* pname,bool dynamic)
	{
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		ASSERT(storagehashmap.find(hash)!=storagehashmap.end(), "Unknown storage slot!");
		return storagehashmap[hash];
		/*uint32_t id = UINT32_MAX;
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
		return id;*/
	}

	void VulkanShader::Rebind(uint32_t* pdynoffsets, uint32_t dynoffcount)
	{
		if (needsRebind) {
			Bind(pdynoffsets, dynoffcount);
			needsRebind = false;
		}
	}
	
}
