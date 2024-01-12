
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include "../../Core/Log.h"
#include "VulkanShader.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "VulkanShaderManager.h"
#include <spirv_reflect.h>


namespace Vulkan {
	
	VulkanShaderManager::VulkanShaderManager(Renderer::RenderDevice*pdevice)
		:_pdevice(pdevice) {
		
	}
	VulkanShaderManager::~VulkanShaderManager() {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		for (auto& pair : _shaderList) {			
			VulkanShaderData& shader = pair.second;
			cleanupBuffer(context.device, shader.uniformBuffer);
			vkDestroyPipelineLayout(context.device, shader.pipelineLayout, nullptr);
			vkDestroyPipeline(context.device, shader.filledPipeline, nullptr);
			vkDestroyPipeline(context.device, shader.wireframePipeline, nullptr);
		}
		//for (auto& pair : _wireframeShaderList) {
		//	VulkanShaderData& shader = pair.second;
		//	//vkDestroyPipelineLayout(context.device, shader.pipelineLayout, nullptr);
		//	vkDestroyPipeline(context.device, shader.pipeline, nullptr);
		//}
	}
	void* VulkanShaderManager::GetShaderDataByName(const char*pname)
	{
		/*assert((size_t)type < _shaderList.size());*/
		ASSERT(_shaderList.find(pname) != _shaderList.end(), "Unknown shader requested!");
		/*if (wireframe) {
			return (void*)&_wireframeShaderList[pname];
		}*/
		return (void*)&_shaderList[pname];
	}
	
	
	uint32_t GetMemberSize(SpvReflectTypeDescription& member) {
		uint32_t size = 0;
		if (member.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
			size= member.traits.numeric.matrix.row_count * member.traits.numeric.matrix.column_count * (member.traits.numeric.scalar.width >> 3);
		}
		else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
			size = member.traits.numeric.vector.component_count * (member.traits.numeric.scalar.width >> 3);
		}
		else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
			size = member.traits.numeric.scalar.width >> 3;
		}
		else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
			size = member.traits.numeric.scalar.width >> 3;
		}
		else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) {
			for (uint32_t i = 0; i < member.member_count; i++) {
				SpvReflectTypeDescription& submember = member.members[i];
				size += GetMemberSize(submember);
			}
		}
		return size;
	}
	void ReflectBlock(SpvReflectBlockVariable& srcblock, VlkBlock& destblock,uint32_t soffset) {
		destblock.members.resize(srcblock.member_count);
		destblock.name = srcblock.type_description->type_name==nullptr ? srcblock.name : srcblock.type_description->type_name;
		destblock.offset = srcblock.offset+soffset;
		destblock.paddedSize = srcblock.padded_size;
		destblock.size = srcblock.size;
		
		uint32_t blocksize = 0;
		for (uint32_t m = 0; m < srcblock.member_count; m++) {
			auto& member = srcblock.members[m];
			uint32_t stride = member.padded_size;
			uint32_t size = member.size;
			uint32_t offset = member.offset+soffset;
			auto& block = destblock.members[m];
			block.name = member.name;
			block.offset = offset;
			block.paddedSize = stride;
			block.size = size;
			if (block.size == 0 && block.paddedSize == 0 && member.type_description->op == SpvOpTypeRuntimeArray) {
				block.size = block.paddedSize = GetMemberSize(*member.type_description);
			}
			block.members.resize(member.member_count);
			blocksize += block.size;
			for (uint32_t m2 = 0; m2 < member.member_count; m2++) {
				ReflectBlock(member.members[m2], block.members[m2],offset);
			}
		}
		if (destblock.paddedSize == 0 && destblock.size==0) {
			destblock.size = destblock.paddedSize = blocksize;
		}
		

	}
	void VulkanShaderManager::Reflect(std::unordered_map < VkShaderStageFlagBits, std::vector<uint32_t>>& spirvMap, ShaderReflection& reflection) {
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<VlkBinding>>> stagebindings;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::tuple<std::string, VkFormat, uint32_t>>> stageInputs;
		std::unordered_map<VkShaderStageFlagBits, VlkPushBlock> pushBlocks;
		for (auto& pair : spirvMap) {
			auto& spirv = pair.second;
			SpvReflectShaderModule module = {};

			SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);

			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			VkShaderStageFlagBits stage;
			switch (module.shader_stage) {
			case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
				stage = VK_SHADER_STAGE_VERTEX_BIT;
				break;
			case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
				stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				break;
			case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
				stage = VK_SHADER_STAGE_GEOMETRY_BIT;
				break;
			case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
				stage = VK_SHADER_STAGE_COMPUTE_BIT;
				break;
			default:
				assert(0);
				break;
			}
			assert(stage == pair.first);

			uint32_t maxSet = 0;
			uint32_t maxPushConst = 0;
			uint32_t descriptorSetCount = module.descriptor_set_count;
			for (uint32_t i = 0; i < descriptorSetCount; i++) {
				SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
				maxSet = std::max(srcset.set + 1, maxSet);
			}

			std::vector<std::vector<VlkBinding>>& descriptorSets = stagebindings[pair.first];
			descriptorSets.resize(maxSet);
			for (uint32_t i = 0; i < descriptorSetCount; i++) {
				SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
				maxSet = std::max(srcset.set + 1, maxSet);

				uint32_t descriptorSetBindingCount = srcset.binding_count;
				auto& bindings = descriptorSets[i];
				bindings.resize(descriptorSetBindingCount);
				//std::vector< DescriptorBinding> descriptorBindings(descriptorSetBindingCount);
				for (uint32_t j = 0; j < descriptorSetBindingCount; j++) {
					SpvReflectDescriptorBinding& srcbinding = *srcset.bindings[j];
					auto& dstbinding = bindings[j];
					dstbinding.binding = srcbinding.binding;
					dstbinding.set = srcbinding.set;
					VkDescriptorType descriptorType = (VkDescriptorType)srcbinding.descriptor_type;
					dstbinding.descriptorType = descriptorType;
					dstbinding.stageFlags = stage;
					dstbinding.count = srcbinding.count;
					dstbinding.name = descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? srcbinding.name : srcbinding.type_description->type_name;
					dstbinding.restype = (VlkResourceType)srcbinding.resource_type;
					uint32_t typeFlags = srcbinding.type_description->type_flags;
					switch (descriptorType) {
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
						ReflectBlock(srcbinding.block, dstbinding.block,0);
					}
																  break;
					case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:

						dstbinding.image.format = (VkFormat)srcbinding.image.image_format;
						dstbinding.image.type = (VkImageType)srcbinding.image.dim;
						dstbinding.image.depth = srcbinding.image.depth;

						break;
					default:
						assert(0);
							break;


					}
				}
			}

			std::vector<std::tuple<std::string, VkFormat, uint32_t>>& inputs = stageInputs[stage];

			uint32_t icount = 0;
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];

				/*uint32_t flags = inputVar.type_description->type_flags;
				if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
					size = inputVar.numeric.vector.component_count * (inputVar.numeric.scalar.width >> 3);
				}*/
				if (inputVar.location > module.input_variable_count && inputVar.built_in)
					continue;//won't happen in vertex shader?
				icount++;
			}
			inputs.resize(icount);
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				uint32_t size = 0;
				/*uint32_t flags = inputVar.type_description->type_flags;
				if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
					size = inputVar.numeric.vector.component_count * (inputVar.numeric.scalar.width >> 3);
				}*/
				if (inputVar.location > module.input_variable_count && inputVar.built_in)
					continue;//won't happen in vertex shader?
				switch (inputVar.format) {
				case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
					size = sizeof(float) * 4;
					break;
				case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
					size = sizeof(float) * 3;
					break;
				case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
					size = sizeof(float) * 2;
					break;
				case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
					size = sizeof(int) * 4;
					break;
				default:
					assert(0);
					break;
				}
				inputs[inputVar.location] = std::tuple<std::string, VkFormat, uint32_t>(inputVar.name, (VkFormat)inputVar.format, size);
			}

			uint32_t pushConstCount = module.push_constant_block_count;
			if (pushConstCount) {
				maxPushConst = std::max(pushConstCount, maxPushConst);

				auto& pb = pushBlocks[stage];
				pb.stageFlags = stage;
				for (uint32_t p = 0; p < pushConstCount; p++) {
					SpvReflectBlockVariable& pc = module.push_constant_blocks[p];
					uint32_t typeFlags = pc.type_description->type_flags;
					uint32_t size = 0;
					pb.name = pc.type_description->type_name;
					if (typeFlags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {
						size = pc.padded_size;
						pb.block.members.resize(pc.member_count);
						pb.block.name = pb.name;
						pb.block.size = pc.size;
						pb.block.paddedSize = pc.padded_size;
						pb.block.offset = pc.offset;
						for (uint32_t m = 0; m < pc.member_count; m++) {
							auto& member = pc.members[m];
							uint32_t stride = member.padded_size;
							uint32_t size = member.size;
							uint32_t offset = member.offset;
							auto& block = pb.block.members[m];
							block.paddedSize = stride;
							block.size = size;
							block.offset = offset;
							block.name = member.name;
						}

					}
					else {
						assert(0);
					}
					pb.size = size;
				}

			}


			spvReflectDestroyShaderModule(&module);

		}

		//merge, munge, etc

		for (auto& pair : pushBlocks) {
			auto& srcblock = pair.second;
			reflection.pushBlock.stageFlags |= pair.first;
			reflection.pushBlock.name = srcblock.name;
			reflection.pushBlock.size = srcblock.size;
			reflection.pushBlock.block = srcblock.block;
		}

		if (stageInputs.find(VK_SHADER_STAGE_VERTEX_BIT) != stageInputs.end()) {
			reflection.inputs = stageInputs[VK_SHADER_STAGE_VERTEX_BIT];
		}
		else if (stageInputs.find(VK_SHADER_STAGE_COMPUTE_BIT) != stageInputs.end()) {
			reflection.inputs = stageInputs[VK_SHADER_STAGE_COMPUTE_BIT];
		}

		//combine bindings
		for (auto& pair : stagebindings) {
			auto stage = pair.first;
			auto& srcbindingsets = pair.second;
			uint32_t maxSet = 0;
			for (auto& srcbindingset : srcbindingsets) {
				for (auto& srcbinding : srcbindingset) {
					maxSet = std::max(maxSet, srcbinding.set + 1);
				}
			}
			if (reflection.bindings.size() < (size_t)maxSet) {
				reflection.bindings.resize(maxSet);
			}
			for (size_t s = 0; s < srcbindingsets.size(); s++) {
				auto& srcbindingset = srcbindingsets[s];
				if (srcbindingset.size() == 0)
					continue;
				uint32_t maxBinding = 0;
				for (auto& srcbinding : srcbindingset) {
					maxBinding = std::max(maxBinding, srcbinding.binding + 1);
				}
				auto& dstbindingset = reflection.bindings[srcbindingset[0].set];
				if (dstbindingset.size() < maxBinding) {
					dstbindingset.resize(maxBinding);
				}
				for (size_t b = 0; b < srcbindingset.size(); b++) {
					auto& srcbinding = srcbindingset[b];
					uint32_t set = srcbinding.set;
					uint32_t binding = srcbinding.binding;

					auto& dstbinding = dstbindingset[binding];
					dstbinding.stageFlags |= stage;
					dstbinding.binding = srcbinding.binding;
					dstbinding.set = srcbinding.set;
					dstbinding.count = srcbinding.count;
					dstbinding.descriptorType = srcbinding.descriptorType;
					if (dstbinding.block.members.size() < srcbinding.block.members.size()) {
						dstbinding.block.members.resize(srcbinding.block.members.size());
					}
					for (size_t i = 0; i < srcbinding.block.members.size(); i++) {
						dstbinding.block.members[i] = srcbinding.block.members[i];
					}
					dstbinding.block.name = srcbinding.block.name;
					dstbinding.block.offset = srcbinding.block.offset;
					dstbinding.block.paddedSize = srcbinding.block.paddedSize;
					dstbinding.block.size = srcbinding.block.size;
					//dstbinding.block = srcbinding.block;
					dstbinding.restype = srcbinding.restype;
					dstbinding.name = srcbinding.name;
					dstbinding.image = srcbinding.image;
				}
			}
		}

	}
	void FlattenBlocks(VlkBlock& block, int setid,int bindingid,std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t,void*>>& blockmembers,int parent) {
		int id = (int)blockmembers.size();
		blockmembers.push_back({ block.name,parent,setid,bindingid,block.offset,block.size,block.paddedSize,nullptr });
		for (auto& member : block.members) {
			FlattenBlocks(member,setid,bindingid, blockmembers, id);
		}
	}
	VkCompareOp VulkanShaderManager::GetCompareOp(Renderer::ShaderCompareOp op) {
		VkCompareOp res = VK_COMPARE_OP_ALWAYS;
		switch (op) {
		case Renderer::ShaderCompareOp::Always:
			res = VK_COMPARE_OP_ALWAYS;
			break;
		case Renderer::ShaderCompareOp::Equal:
			res = VK_COMPARE_OP_EQUAL;
			break;
		case Renderer::ShaderCompareOp::Greater:
			res = VK_COMPARE_OP_GREATER;
			break;
		case Renderer::ShaderCompareOp::GreaterOrEqual:
			res = VK_COMPARE_OP_GREATER_OR_EQUAL;
			break;
		case Renderer::ShaderCompareOp::Less:
			res = VK_COMPARE_OP_LESS;
			break;
		case Renderer::ShaderCompareOp::LessOrEqual:
			res = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;
		case Renderer::ShaderCompareOp::Never:
			res = VK_COMPARE_OP_NEVER;
			break;
		case Renderer::ShaderCompareOp::NotEqual:
			res = VK_COMPARE_OP_NOT_EQUAL;
			break;
		}
		return res;
	}
	VkBlendFactor VulkanShaderManager::GetBlendFactor(Renderer::ShaderBlendFactor factor) {
		VkBlendFactor res = VK_BLEND_FACTOR_MAX_ENUM;
		switch (factor) {
		case Renderer::ShaderBlendFactor::ConstantAlpha:
			res = VK_BLEND_FACTOR_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::ConstantColor:
			res = VK_BLEND_FACTOR_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::DstAlpha:
			res = VK_BLEND_FACTOR_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::DstColor:
			res = VK_BLEND_FACTOR_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::One:
			res = VK_BLEND_FACTOR_ONE;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantAlpha:
			res = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantColor:
			res = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstAlpha:
			res = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstColor:
			res = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcAlpha:
			res = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcColor:
			res = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::SrcAlpha:
			res = VK_BLEND_FACTOR_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::SrcColor:
			res = VK_BLEND_FACTOR_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::Zero:
			res = VK_BLEND_FACTOR_ZERO;
			break;
		}
		return res;
	}
	VkBlendOp VulkanShaderManager::GetBlendOp(Renderer::ShaderBlendOp op) {
		VkBlendOp res = VK_BLEND_OP_MAX_ENUM;
		switch (op) {
		case Renderer::ShaderBlendOp::Add:
			res = VK_BLEND_OP_ADD;
			break;
		case Renderer::ShaderBlendOp::Max:
			res = VK_BLEND_OP_MAX;
			break;
		case Renderer::ShaderBlendOp::Min:
			res = VK_BLEND_OP_MIN;
			break;
		case Renderer::ShaderBlendOp::ReverseSubstract:
			res = VK_BLEND_OP_REVERSE_SUBTRACT;
			break;
		case Renderer::ShaderBlendOp::Subtract:
			res = VK_BLEND_OP_SUBTRACT;
			break;		
		}
		return res;
	}
	VkCullModeFlagBits VulkanShaderManager::GetCullMode(Renderer::ShaderCullMode mode) {
		VkCullModeFlagBits res = VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
		switch (mode) {
			
		case Renderer::ShaderCullMode::backFace:
			res = VK_CULL_MODE_BACK_BIT;
			break;
		case Renderer::ShaderCullMode::frontFace:
			res = VK_CULL_MODE_FRONT_BIT;
			break;
		case Renderer::ShaderCullMode::frontandbackFace:
			res = VK_CULL_MODE_FRONT_AND_BACK;
			break;
		case Renderer::ShaderCullMode::None:
			res = VK_CULL_MODE_NONE;
			break;
		}
		return res;
	}
	void VulkanShaderManager::CompileShader(const std::string& name, const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources, Renderer::ShaderCreateInfo& createInfo) {
		LOG_INFO("Compiling shader {0}", name);
		ShaderCompiler compiler;
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> spirvMap;
		for (auto& pair : shaderSources) {
			std::vector<uint32_t> spirv = compiler.compileShader(pair.second.c_str(), pair.first);
			if (spirv.size() == 0) {
				LOG_ERROR("Unable to compile shader: {0}", pair.first);
			}
			spirvMap[pair.first] = spirv;
		}

		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<VkDescriptorSetLayoutBinding>>> shaderBindings;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<std::string>>> shaderBindingNames;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::vector<std::string>>> shaderBindingCombinedNames;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::vector<uint32_t>>> shaderBindingCombinedOffsets;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<uint32_t>>> shaderBindingSizes;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::tuple<std::string, VkFormat, uint32_t>>> shaderInputs;
		std::unordered_map<VkShaderStageFlagBits, std::vector<VkPushConstantRange>> pushConstRanges;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::string>> pushConstNames;
		uint32_t maxSet = 0;
		uint32_t maxPushConst = 0;

		ShaderReflection& reflection = _shaderList[name].reflection;
		Reflect(spirvMap, reflection);
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>>& blockmembers = reflection.blockmembers;
		std::unordered_map<size_t, int>& blockmap = reflection.blockmap;
		for (auto& bindingset : reflection.bindings) {
			for (auto& binding : bindingset) {
				int resType = (int)binding.restype;
				if (resType & (int)VlkResourceType::Sampler) {
					blockmembers.push_back({ binding.name,-1,binding.set , binding.binding,binding.count,0,0,nullptr });
				}
				else {
					FlattenBlocks(binding.block, binding.set, binding.binding, blockmembers, -1);
				}
			}
		}
		if (reflection.pushBlock.block.members.size() > 0)
			FlattenBlocks(reflection.pushBlock.block, -1, -1, blockmembers, -1);
		for (size_t i = 0; i < blockmembers.size(); i++) {
			auto& tup = blockmembers[i];
			std::string str = std::get<0>(tup);
			int parent = std::get<1>(tup);
			std::vector<std::string> list = { str };
			std::vector<std::string> names = { str };
			while (parent != -1) {
				str = std::get<0>(blockmembers[parent]);
				parent = std::get<1>(blockmembers[parent]);
				list.push_back(str);
				std::string fullname;
				for (auto& name : list) {
					if (fullname.empty())
						fullname = name;
					else fullname = name + "." + fullname;
					if (std::find(names.begin(), names.end(), fullname) == names.end()) {
						names.push_back(fullname);
					}
				}
			}
			for (auto& name : names) {
				size_t hash = Core::HashFNV1A(name.c_str(), name.length());
				assert(blockmap.find(hash) == blockmap.end());
				blockmap[hash] = (int)i;
			}
		}
		auto ptypes = createInfo.ptypes;
		auto numtypes = createInfo.numtypes;
		if (ptypes && numtypes > 0)
		{
			uint32_t typeIndex = 0;
			//change dynamic types

			for (auto& set : reflection.bindings) {
				for (auto& binding : set) {
					VkDescriptorType descriptorType = binding.descriptorType;

					if (typeIndex > numtypes) {
						LOG_WARN("More storage types {0} in shader than expected {1}.", typeIndex, numtypes);
					}
					switch (ptypes[typeIndex]) {
					case Renderer::ShaderStorageType::Uniform:
						if (descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							LOG_WARN("Expected Uniform at {0}", typeIndex);
						break;
					case Renderer::ShaderStorageType::UniformDynamic:
						if (!(descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC))
							LOG_WARN("Expected Dynamic Uniform at {0}", typeIndex);
						descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
						break;
					case Renderer::ShaderStorageType::Storage:
						if (descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
							LOG_WARN("Expected Storage buffer at {0}", typeIndex);
						break;
					case Renderer::ShaderStorageType::StorageDynamic:
						if (!(descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))
							LOG_WARN("Expected Dynamic Storage buffer at {0}", typeIndex);
						descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
						break;
					case Renderer::ShaderStorageType::Texture:
						if (descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
							LOG_WARN("Expected Combined image sampler at {0}", typeIndex);
						break;
					default:
						LOG_WARN("Unexpected shader type at {0}", typeIndex);
						break;
					}

					typeIndex++;
					binding.descriptorType = descriptorType;
				}

			}
		}
		{
			Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
			Vulkan::VulkContext& context = *contextptr;
			Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
			Vulkan::VulkFrameData& framedata = *framedataptr;
			VkRenderPass renderPass = framedata.renderPass;
			if (createInfo.platformData) {
				renderPass = (*(VkRenderPass*)createInfo.platformData);
			}
			//build descriptor sets
			std::vector<VkDescriptorSetLayout> layouts;
			for (auto& set : reflection.bindings) {
				auto layoutbuilder = DescriptorSetLayoutBuilder::begin(context.pLayoutCache);
				for (auto& binding : set) {
					layoutbuilder.AddBinding(binding.getBinding());
				}
				VkDescriptorSetLayout descriptorLayout = layoutbuilder.build();
				layouts.push_back(descriptorLayout);

			}
			std::vector<VkPushConstantRange> pushConstRanges;
			if (reflection.pushBlock.size > 0)
				pushConstRanges.push_back({ reflection.pushBlock.stageFlags,0,reflection.pushBlock.size });

			VkPipelineLayout pipelineLayout;
			PipelineLayoutBuilder::begin(context.device)
				.AddDescriptorSetLayouts(layouts)
				.AddPushConstants(pushConstRanges)
				.build(pipelineLayout);

			std::vector<ShaderModule> shaders;
			for (auto& pair : spirvMap) {
				auto& spirv = pair.second;
				VkShaderModule shader = VK_NULL_HANDLE;

				VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
				createInfo.codeSize = spirv.size() * sizeof(uint32_t);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
				VkResult res = vkCreateShaderModule(context.device, &createInfo, nullptr, &shader);
				ASSERT(res == VK_SUCCESS, "Unable to create shader module!");
				shaders.push_back({ shader,pair.first });
			}
			VkVertexInputBindingDescription vertexInputDescription;

			auto& vertexInputs = reflection.inputs;// shaderInputs[VK_SHADER_STAGE_VERTEX_BIT];
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions(vertexInputs.size());
			uint32_t offset = 0;
			for (size_t i = 0; i < vertexInputs.size(); i++) {
				auto& tuple = vertexInputs[i];
				VkFormat format = std::get<1>(tuple);
				uint32_t size = std::get<2>(tuple);

				vertexAttributeDescriptions[i].location = (uint32_t)i;
				vertexAttributeDescriptions[i].offset = offset;
				vertexAttributeDescriptions[i].format = format;
				offset += size;
			}
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = offset;
			vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			{
				VkPipeline pipeline = VK_NULL_HANDLE;
				PipelineBuilder::begin(context.device, pipelineLayout, renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(createInfo.blendInfo.enable? VK_TRUE : VK_FALSE)
					.setDepthTest(createInfo.depthInfo.enable ? VK_TRUE : VK_FALSE)
					.setDepthCompareOp(GetCompareOp(createInfo.depthInfo.compare))
					.setCullMode(GetCullMode(createInfo.cullMode))//cullMode == Renderer::ShaderCullMode::frontFace ? VK_CULL_MODE_FRONT_BIT : cullMode == Renderer::ShaderCullMode::backFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE)
					.setBlendState(GetBlendFactor(createInfo.blendInfo.srcColorFactor),GetBlendFactor(createInfo.blendInfo.dstColorFactor),GetBlendOp(createInfo.blendInfo.colorOp),GetBlendFactor(createInfo.blendInfo.srcAlphaFactor),GetBlendFactor(createInfo.blendInfo.dstAlphaFactor),GetBlendOp(createInfo.blendInfo.alphaOp))					
					.build(pipeline);
				auto& shaderData = _shaderList[name];
				shaderData.descriptorSetLayouts = layouts;

				shaderData.pipeline = shaderData.filledPipeline = pipeline;
				shaderData.pipelineLayout = pipelineLayout;

			}
			{
				VkPipeline pipeline = VK_NULL_HANDLE;
				//wireframe
				PipelineBuilder::begin(context.device, pipelineLayout, renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setPolygonMode(VK_POLYGON_MODE_LINE)
					.setBlend(createInfo.blendInfo.enable ? VK_TRUE : VK_FALSE)
					.setDepthTest(createInfo.depthInfo.enable ? VK_TRUE : VK_FALSE)
					.setDepthCompareOp(GetCompareOp(createInfo.depthInfo.compare))
					.setCullMode(GetCullMode(createInfo.cullMode))//cullMode == Renderer::ShaderCullMode::frontFace ? VK_CULL_MODE_FRONT_BIT : cullMode == Renderer::ShaderCullMode::backFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE)
					.setBlendState(GetBlendFactor(createInfo.blendInfo.srcColorFactor), GetBlendFactor(createInfo.blendInfo.dstColorFactor), GetBlendOp(createInfo.blendInfo.colorOp), GetBlendFactor(createInfo.blendInfo.srcAlphaFactor), GetBlendFactor(createInfo.blendInfo.dstAlphaFactor), GetBlendOp(createInfo.blendInfo.alphaOp))
					.build(pipeline);
				_shaderList[name].wireframePipeline = pipeline;


			}
			for (auto& shader : shaders) {
				cleanupShaderModule(context.device, shader.shaderModule);
			}


			//allocate uniforms, TODO: don't allocate for each shader, as could be 1 uniform buffer used by many shaders.
			std::vector<UniformBufferInfo> bufferInfo;
			std::vector<std::pair<size_t, size_t>> bindsets;
			Vulkan::Buffer uniformBuffer;
			UniformBufferBuilder builder = UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true);
			bool hasUniform = false;
			for (size_t s = 0; s < reflection.bindings.size(); s++) {
				auto& bindingset = reflection.bindings[s];
				for (size_t b = 0; b < bindingset.size(); b++) {
					auto& binding = bindingset[b];
					if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
						hasUniform = true;
						builder.AddBuffer(binding.getPaddedSize(), 1, 1);
						bindsets.push_back(std::make_pair(s, b));
					}
				}
			}
			if (hasUniform) {//create uniform buffers and set pointer in table to memory
				builder.build(uniformBuffer, bufferInfo);
				_shaderList[name].uniformBuffer = uniformBuffer;
				for (size_t i = 0; i < bindsets.size(); i++) {
					int s = (int)bindsets[i].first;
					int b = (int)bindsets[i].second;
					for (size_t j = 0; j < reflection.blockmembers.size(); j++) {
						//int p = std::get<1>(reflection.blockmembers[j]);
						int s2 = std::get<2>(reflection.blockmembers[j]);
						int b2 = std::get<3>(reflection.blockmembers[j]);
						uint32_t offset = std::get<4>(reflection.blockmembers[j]);
						if (s == s2 && b == b2) {
							std::get<7>(reflection.blockmembers[j]) = (void*)((uint8_t*)bufferInfo[i].ptr + offset);

						}
					}
				}
			}




		}
	}
	void VulkanShaderManager::CompileShader(const std::string&name,const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources,Renderer::ShaderCullMode cullMode, bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes, void* platformData)
	{
		LOG_INFO("Compiling shader {0}", name);
		ShaderCompiler compiler;
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> spirvMap;
		for (auto &pair : shaderSources) {
			std::vector<uint32_t> spirv = compiler.compileShader(pair.second.c_str(),pair.first);
			if (spirv.size() == 0) {
				LOG_ERROR("Unable to compile shader: {0}", pair.first);
			}
			spirvMap[pair.first] = spirv;
		}
		////get descriptor sets, ubo sizes, etc
		//struct DescrBase {
		//	VkDescriptorSetLayoutBinding binding;
		//	VkDescriptorType type;		
		//	std::string name;
		//	std::vector<std::string> membernames;
		//	std::vector<uint32_t> memberoffsets;
		//	std::vector<uint32_t> membersizes;
		//	std::vector<uint32_t> memberstrides;
		//};
		
		
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<VkDescriptorSetLayoutBinding>>> shaderBindings;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<std::string>>> shaderBindingNames;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::vector<std::string>>> shaderBindingCombinedNames;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::vector<uint32_t>>> shaderBindingCombinedOffsets;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<uint32_t>>> shaderBindingSizes;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::tuple<std::string, VkFormat, uint32_t>>> shaderInputs;
		std::unordered_map<VkShaderStageFlagBits, std::vector<VkPushConstantRange>> pushConstRanges;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::string>> pushConstNames;
		uint32_t maxSet = 0;
		uint32_t maxPushConst = 0;
		
		ShaderReflection& reflection = _shaderList[name].reflection;
		Reflect(spirvMap, reflection);
		std::vector<std::tuple<std::string, int, int,int, uint32_t, uint32_t, uint32_t,void*>> &blockmembers=reflection.blockmembers;
		std::unordered_map<size_t, int>& blockmap=reflection.blockmap;
		for (auto& bindingset : reflection.bindings) {
			for (auto& binding : bindingset) {
				int resType = (int)binding.restype;
				if (resType & (int)VlkResourceType::Sampler) {
					blockmembers.push_back({ binding.name,-1,binding.set , binding.binding,binding.count,0,0,nullptr });
				}
				else {
					FlattenBlocks(binding.block, binding.set, binding.binding, blockmembers, -1);
				}
			}
		}
		if(reflection.pushBlock.block.members.size()>0)
			FlattenBlocks(reflection.pushBlock.block, -1, -1, blockmembers, -1);
		for (size_t i = 0;i< blockmembers.size();i++) {
			auto& tup = blockmembers[i];
			std::string str = std::get<0>(tup);
			int parent = std::get<1>(tup);
			std::vector<std::string> list = { str };
			std::vector<std::string> names = { str };
			while (parent != -1) {
				str = std::get<0>(blockmembers[parent]);
				parent = std::get<1>(blockmembers[parent]);
				list.push_back(str);
				std::string fullname;
				for (auto& name : list) {
					if (fullname.empty())
						fullname = name;
					else fullname = name + "." + fullname;
					if (std::find(names.begin(), names.end(), fullname) == names.end()) {
						names.push_back(fullname);
					}
				}
			}
			for (auto& name : names) {
				size_t hash = Core::HashFNV1A(name.c_str(), name.length());
				assert(blockmap.find(hash) == blockmap.end());
				blockmap[hash] = (int)i;
			}
		}
		if (ptypes && numtypes > 0)
		{
			uint32_t typeIndex = 0;
			//change dynamic types

			for (auto& set : reflection.bindings) {
				for (auto& binding : set) {
					VkDescriptorType descriptorType = binding.descriptorType;

					if (typeIndex > numtypes) {
						LOG_WARN("More storage types {0} in shader than expected {1}.", typeIndex, numtypes);
					}
					switch (ptypes[typeIndex]) {
					case Renderer::ShaderStorageType::Uniform:
						if (descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
							LOG_WARN("Expected Uniform at {0}", typeIndex);
						break;
					case Renderer::ShaderStorageType::UniformDynamic:
						if (!(descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC))
							LOG_WARN("Expected Dynamic Uniform at {0}", typeIndex);
						descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
						break;
					case Renderer::ShaderStorageType::Storage:
						if (descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
							LOG_WARN("Expected Storage buffer at {0}", typeIndex);
						break;
					case Renderer::ShaderStorageType::StorageDynamic:
						if (!(descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))
							LOG_WARN("Expected Dynamic Storage buffer at {0}", typeIndex);
						descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
						break;
					case Renderer::ShaderStorageType::Texture:
						if (descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
							LOG_WARN("Expected Combined image sampler at {0}", typeIndex);
						break;
					default:
						LOG_WARN("Unexpected shader type at {0}", typeIndex);
						break;
					}

					typeIndex++;
					binding.descriptorType = descriptorType;
				}

			}
		}
		{
			Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
			Vulkan::VulkContext& context = *contextptr;
			Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
			Vulkan::VulkFrameData& framedata = *framedataptr;
			VkRenderPass renderPass = framedata.renderPass;
			if (platformData) {
				renderPass = (*(VkRenderPass*)platformData);
			}
			//build descriptor sets
			std::vector<VkDescriptorSetLayout> layouts;
			for (auto& set : reflection.bindings) {
				auto layoutbuilder = DescriptorSetLayoutBuilder::begin(context.pLayoutCache);
				for (auto& binding : set) {
					layoutbuilder.AddBinding(binding.getBinding());
				}
				VkDescriptorSetLayout descriptorLayout = layoutbuilder.build();
				layouts.push_back(descriptorLayout);

			}
			std::vector<VkPushConstantRange> pushConstRanges;
			if(reflection.pushBlock.size>0)
				pushConstRanges.push_back({reflection.pushBlock.stageFlags,0,reflection.pushBlock.size});

			VkPipelineLayout pipelineLayout;
			PipelineLayoutBuilder::begin(context.device)
				.AddDescriptorSetLayouts(layouts)
				.AddPushConstants(pushConstRanges)
				.build(pipelineLayout);

			std::vector<ShaderModule> shaders;
			for (auto& pair : spirvMap) {
				auto& spirv = pair.second;
				VkShaderModule shader = VK_NULL_HANDLE;

				VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
				createInfo.codeSize = spirv.size() * sizeof(uint32_t);
				createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
				VkResult res = vkCreateShaderModule(context.device, &createInfo, nullptr, &shader);
				ASSERT(res == VK_SUCCESS, "Unable to create shader module!");
				shaders.push_back({ shader,pair.first });
			}
			VkVertexInputBindingDescription vertexInputDescription;

			auto& vertexInputs = reflection.inputs;// shaderInputs[VK_SHADER_STAGE_VERTEX_BIT];
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions(vertexInputs.size());
			uint32_t offset = 0;
			for (size_t i = 0; i < vertexInputs.size(); i++) {
				auto& tuple = vertexInputs[i];
				VkFormat format = std::get<1>(tuple);
				uint32_t size = std::get<2>(tuple);

				vertexAttributeDescriptions[i].location = (uint32_t)i;
				vertexAttributeDescriptions[i].offset = offset;
				vertexAttributeDescriptions[i].format = format;
				offset += size;
			}
			vertexInputDescription.binding = 0;
			vertexInputDescription.stride = offset;
			vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			{
				VkPipeline pipeline = VK_NULL_HANDLE;
				PipelineBuilder::begin(context.device, pipelineLayout, renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(enableBlend ? VK_TRUE : VK_FALSE)
					.setDepthTest(enableDepth ? VK_TRUE : VK_FALSE)
					.setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
					.setCullMode(cullMode==Renderer::ShaderCullMode::frontFace ? VK_CULL_MODE_FRONT_BIT : cullMode == Renderer::ShaderCullMode::backFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE)
					//.setDepthTest(VK_TRUE)//need this to be a parameter
					.build(pipeline);
				auto& shaderData = _shaderList[name];
				shaderData.descriptorSetLayouts = layouts;

				shaderData.pipeline = shaderData.filledPipeline = pipeline;
				shaderData.pipelineLayout = pipelineLayout;
				
			}
			{
				VkPipeline pipeline = VK_NULL_HANDLE;
				//wireframe
				PipelineBuilder::begin(context.device, pipelineLayout, renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setPolygonMode(VK_POLYGON_MODE_LINE)
					.setCullMode(cullMode == Renderer::ShaderCullMode::frontFace ? VK_CULL_MODE_FRONT_BIT : cullMode == Renderer::ShaderCullMode::backFace ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE)
					//.setDepthTest(VK_TRUE)//need this to be a parameter
					.build(pipeline);
				_shaderList[name].wireframePipeline = pipeline;


			}
			for (auto& shader : shaders) {
				cleanupShaderModule(context.device, shader.shaderModule);
			}


			//allocate uniforms, TODO: don't allocate for each shader, as could be 1 uniform buffer used by many shaders.
			std::vector<UniformBufferInfo> bufferInfo;
			std::vector<std::pair<size_t, size_t>> bindsets;
			Vulkan::Buffer uniformBuffer;
			UniformBufferBuilder builder = UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true);
			bool hasUniform = false;
			for (size_t s = 0; s < reflection.bindings.size(); s++) {
				auto& bindingset = reflection.bindings[s];
				for (size_t b = 0; b < bindingset.size(); b++) {
					auto& binding = bindingset[b];
					if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
						hasUniform = true;
						builder.AddBuffer(binding.getPaddedSize(), 1, 1);
						bindsets.push_back(std::make_pair(s, b));
					}
				}
			}
			if (hasUniform) {//create uniform buffers and set pointer in table to memory
				builder.build(uniformBuffer, bufferInfo);
				_shaderList[name].uniformBuffer = uniformBuffer;
				for (size_t i = 0; i < bindsets.size(); i++) {
					int s = (int)bindsets[i].first;
					int b = (int)bindsets[i].second;
					for (size_t j = 0; j < reflection.blockmembers.size(); j++) {
						//int p = std::get<1>(reflection.blockmembers[j]);
						int s2 = std::get<2>(reflection.blockmembers[j]);
						int b2 = std::get<3>(reflection.blockmembers[j]);
						uint32_t offset = std::get<4>(reflection.blockmembers[j]);
						if ( s == s2 && b == b2) {
							std::get<7>(reflection.blockmembers[j]) = (void*)((uint8_t*)bufferInfo[i].ptr + offset);
							
						}
					}
				}
			}




		}
		


	}
	
	std::string VulkanShaderManager::readFile(const std::string& filepath) {//borowed from TheCherno Hazel shader stuff
		std::string result;		
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		ASSERT(in, "Unable to open shader file.");
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());
		in.close();
		return result;
	}

	std::unordered_map<VkShaderStageFlagBits, std::string> VulkanShaderManager::PreProcess(const std::string& source) {
		std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			ASSERT(pos != std::string::npos, "Syntax error reading shader file.");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			VkShaderStageFlagBits stage = ShaderTypeFromString(type);
			ASSERT(stage != VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM, "Invalid shader type specified!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[stage] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));

		}
		return shaderSources;
	}
	VkShaderStageFlagBits VulkanShaderManager::ShaderTypeFromString(const std::string& type) {
		if (type == "vertex")
			return VK_SHADER_STAGE_VERTEX_BIT;
		else if (type == "geometry")
			return VK_SHADER_STAGE_VERTEX_BIT;
		else if (type == "fragment")
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
	void* VulkanShaderManager::CreateShaderData(const char* shaderPath,Renderer::ShaderCullMode cullMode,bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes, void* platformData) {
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::string source = readFile(filepath);
			const std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources = PreProcess(source);
			CompileShader(name,shaderSources,cullMode,enableBlend,enableDepth,ptypes,numtypes,platformData);
		}
		return &_shaderList[name];
	}

	void* VulkanShaderManager::CreateShaderData(const char* shaderPath, Renderer::ShaderCreateInfo& info) {
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::string source = readFile(filepath);
			const std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources = PreProcess(source);
			CompileShader(name, shaderSources, info);
		}
		return &_shaderList[name];
	}
}