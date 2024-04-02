#include "SPIRVReflector.h"
#include "../../Core/Log.h"
#include <cassert>
#include <spirv_reflect.h>
namespace SPIRVUtils {
	SPIRVReflector::SPIRVReflector() {

	}

	SPIRVReflector::~SPIRVReflector() {

	}

	uint32_t getMemberSize(SpvReflectTypeDescription& member) {
		uint32_t size = 0;
		if (member.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
			size = member.traits.numeric.matrix.row_count * member.traits.numeric.matrix.column_count * (member.traits.numeric.scalar.width >> 3);
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
				size += getMemberSize(submember);
			}
		}
		return size;
	}

	
	

	void reflectBlock(SpvReflectBlockVariable& srcblock, SPIRVBlock& destblock, uint32_t soffset) {
		destblock.members.resize(srcblock.member_count);
		destblock.name = srcblock.type_description->type_name == nullptr ? srcblock.name : srcblock.type_description->type_name;
		destblock.offset = srcblock.offset + soffset;
		destblock.paddedSize = srcblock.padded_size;
		destblock.size = srcblock.size;

		uint32_t blocksize = 0;
		for (uint32_t m = 0; m < srcblock.member_count; m++) {
			auto& member = srcblock.members[m];
			uint32_t stride = member.padded_size;
			uint32_t size = member.size;
			uint32_t offset = member.offset + soffset;
			auto& block = destblock.members[m];
			block.name = member.name;
			block.offset = offset;
			block.paddedSize = stride;
			block.size = size;
			if (block.size == 0 && block.paddedSize == 0 && member.type_description->op == SpvOpTypeRuntimeArray) {
				block.size = block.paddedSize = getMemberSize(*member.type_description);
			}
			block.members.resize(member.member_count);
			blocksize += block.size;
			for (uint32_t m2 = 0; m2 < member.member_count; m2++) {
				reflectBlock(member.members[m2], block.members[m2], offset);
			}
		}
		if (destblock.paddedSize == 0 && destblock.size == 0) {
			destblock.size = destblock.paddedSize = blocksize;
		}
	}

	void SPIRVReflector::reflect(std::unordered_map<SPIRVShaderStage, std::vector<uint32_t>>& spirvMap, SPIRVShaderReflection& reflection) {
		std::unordered_map <SPIRVShaderStage, std::vector<std::vector<SPIRVBinding>>> stagebindings;
		std::unordered_map<SPIRVShaderStage, std::vector<std::tuple<std::string, SPIRVFormat, uint32_t>>> stageInputs;
		
		std::unordered_map<SPIRVShaderStage, SPIRVPushBlock> pushBlocks;
		for (auto& pair : spirvMap) {
			auto& spirv = pair.second;
			SpvReflectShaderModule module = {};

			SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);

			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			SPIRVShaderStage stage = (SPIRVShaderStage)module.shader_stage;

			
			assert(stage == pair.first);

			uint32_t maxSet = 0;
			uint32_t maxPushConst = 0;
			uint32_t descriptorSetCount = module.descriptor_set_count;
			for (uint32_t i = 0; i < descriptorSetCount; i++) {
				SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
				maxSet = std::max(srcset.set + 1, maxSet);
			}

			std::vector<std::vector<SPIRVBinding>>& descriptorSets = stagebindings[pair.first];
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
					auto descriptorType = srcbinding.descriptor_type;
					dstbinding.descriptorType = (SPIRVDescriptorType)descriptorType;
					dstbinding.stageFlags = stage;
					dstbinding.count = srcbinding.count;
					dstbinding.name = descriptorType == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? srcbinding.name : srcbinding.type_description->type_name;
					dstbinding.restype = (SPIRVResourceType)srcbinding.resource_type;
					uint32_t typeFlags = srcbinding.type_description->type_flags;
					switch (descriptorType) {
					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
						reflectBlock(srcbinding.block, dstbinding.block, 0);
					}
																  break;
					case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:

						dstbinding.image.format = (SPIRVImageFormat)srcbinding.image.image_format;
						dstbinding.image.type = (SPIRVImageType)srcbinding.image.dim;
						dstbinding.image.depth = srcbinding.image.depth;

						break;
					default:
						assert(0);
						break;


					}
				}
			}

			std::vector<std::tuple<std::string, SPIRVUtils::SPIRVFormat, uint32_t>>& inputs = stageInputs[stage];
			
			uint32_t icount = 0;
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				if (inputVar.location > module.input_variable_count && inputVar.built_in)
					continue;//won't happen in vertex shader?
				icount++;
			}
			inputs.resize(icount);
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];
				
				uint32_t size = 0;
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
				case SPV_REFLECT_FORMAT_UNDEFINED://probably a structure of inputs
					size = 0;//could break this into a block type, but probably wouldn't be necessary
					break;
				default:
					assert(0);
					break;
				}
				inputs[inputVar.location] = std::tuple<std::string, SPIRVUtils::SPIRVFormat, uint32_t>(inputVar.name, (SPIRVUtils::SPIRVFormat)inputVar.format, size);
				
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
			int stageFlags = (int)reflection.pushBlock.stageFlags;
			stageFlags |= (int)pair.first;
			reflection.pushBlock.stageFlags = (SPIRVUtils::SPIRVShaderStage)stageFlags;
			reflection.pushBlock.name = srcblock.name;
			reflection.pushBlock.size = srcblock.size;
			reflection.pushBlock.block = srcblock.block;
		}

		if (stageInputs.find(SPIRVUtils::SPIRVShaderStage::Vertex) != stageInputs.end()) {
			reflection.inputs = stageInputs[SPIRVUtils::SPIRVShaderStage::Vertex];
		}
		else if (stageInputs.find(SPIRVUtils::SPIRVShaderStage::Compute) != stageInputs.end()) {
			reflection.inputs = stageInputs[SPIRVUtils::SPIRVShaderStage::Compute];
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
				//uint32_t maxBinding = 0;
				//for (auto& srcbinding : srcbindingset) {
				//	maxBinding = std::max(maxBinding, srcbinding.binding + 1);//can introduce empty slots, so need to remove later
				//}
				auto& dstbindingset = reflection.bindings[srcbindingset[0].set];
				//
				//if (dstbindingset.size() < maxBinding) {

				//	dstbindingset.resize(maxBinding);
				//}
				for (size_t b = 0; b < srcbindingset.size(); b++) {
					auto& srcbinding = srcbindingset[b];
					uint32_t set = srcbinding.set;
					uint32_t binding = srcbinding.binding;
					uint32_t index = UINT32_MAX;
					auto descriptorType = srcbinding.descriptorType;
					for (size_t i = 0; i < dstbindingset.size(); i++) {
						auto& dstbinding = dstbindingset[i];
						if (dstbinding.binding == binding && dstbinding.descriptorType == descriptorType) {
							//alreay exists?
							index = (uint32_t)i;
							break;
						}
					}
					int bindx = binding;
					if (index == UINT32_MAX) {
						dstbindingset.push_back({});
						bindx = dstbindingset.size() - 1;
					}
					auto& dstbinding = dstbindingset[bindx];
					int stageFlags = (int)dstbinding.stageFlags;
					stageFlags |= (int)stage;
					dstbinding.stageFlags = (SPIRVUtils::SPIRVShaderStage)stageFlags;
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
		//check for empty binding
		for (size_t s = 0; s < reflection.bindings.size(); s++) {
			auto& bindingset = reflection.bindings[s];
			std::vector<SPIRVBinding> newbinding;
			for (size_t i = 0; i < bindingset.size(); i++) {
				if (bindingset[i].descriptorType != SPIRVUtils::SPIRVDescriptorType::MaxEnum) {
					newbinding.push_back(bindingset[i]);
				}
			}
			if (newbinding.size() < bindingset.size()) {
				reflection.bindings[s] = newbinding;
			}
		}
	}
}