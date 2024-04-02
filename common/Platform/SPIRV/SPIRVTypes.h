#pragma once

namespace SPIRVUtils{
	enum class SPIRVSemantics { OpenGL, Vulkan,/*HLSL*/ };
	enum class SPIRVShaderStage {
		Undefined = 0,
		Vertex = 0x00000001, // = VK_SHADER_STAGE_VERTEX_BIT
		TessellationControl = 0x00000002, // = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT
		TessellationEvaluation = 0x00000004, // = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
		Geometry = 0x00000008, // = VK_SHADER_STAGE_GEOMETRY_BIT
		Fragment = 0x00000010, // = VK_SHADER_STAGE_FRAGMENT_BIT
		Compute = 0x00000020, // = VK_SHADER_STAGE_COMPUTE_BIT
		TASK_NV = 0x00000040, // = VK_SHADER_STAGE_TASK_BIT_NV
		TASK_EXT = TASK_NV, // = VK_SHADER_STAGE_CALLABLE_EXT
		MESH_NV = 0x00000080, // = VK_SHADER_STAGE_MESH_BIT_NV
		MESH_EXT = MESH_NV, // = VK_SHADER_STAGE_CALLABLE_BIT_EXT
		RAYGEN_KHR = 0x00000100, // = VK_SHADER_STAGE_RAYGEN_BIT_KHR
		ANY_HIT_KHR = 0x00000200, // = VK_SHADER_STAGE_ANY_HIT_BIT_KHR
		CLOSEST_HIT_KHR = 0x00000400, // = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR
		MISS_KHR = 0x00000800, // = VK_SHADER_STAGE_MISS_BIT_KHR
		INTERSECTION_KHR = 0x00001000, // = VK_SHADER_STAGE_INTERSECTION_BIT_KHR
		CALLABLE_KHR = 0x00002000, // = VK_SHADER_STAGE_CALLABLE_BIT_KHR 
		MaxEnum
	};
	enum class SPIRVImageFormat {
		Unknown = 0,
		Rgba32f = 1,
		Rgba16f = 2,
		R32f = 3,
		Rgba8 = 4,
		Rgba8Snorm = 5,
		Rg32f = 6,
		Rg16f = 7,
		R11fG11fB10f = 8,
		R16f = 9,
		Rgba16 = 10,
		Rgb10A2 = 11,
		Rg16 = 12,
		Rg8 = 13,
		R16 = 14,
		R8 = 15,
		Rgba16Snorm = 16,
		Rg16Snorm = 17,
		Rg8Snorm = 18,
		R16Snorm = 19,
		R8Snorm = 20,
		Rgba32i = 21,
		Rgba16i = 22,
		Rgba8i = 23,
		R32i = 24,
		Rg32i = 25,
		Rg16i = 26,
		Rg8i = 27,
		R16i = 28,
		R8i = 29,
		Rgba32ui = 30,
		Rgba16ui = 31,
		Rgba8ui = 32,
		R32ui = 33,
		Rgb10a2ui = 34,
		Rg32ui = 35,
		Rg16ui = 36,
		Rg8ui = 37,
		R16ui = 38,
		R8ui = 39,
		R64ui = 40,
		R64i = 41,
		MaxEnum = 0x7fffffff,
	};
	enum class SPIRVFormat {
		UNDEFINED = 0, // = VK_FORMAT_UNDEFINED
		R16_UINT = 74, // = VK_FORMAT_R16_UINT
		R16_SINT = 75, // = VK_FORMAT_R16_SINT
		R16_SFLOAT = 76, // = VK_FORMAT_R16_SFLOAT
		R16G16_UINT = 81, // = VK_FORMAT_R16G16_UINT
		R16G16_SINT = 82, // = VK_FORMAT_R16G16_SINT
		R16G16_SFLOAT = 83, // = VK_FORMAT_R16G16_SFLOAT
		R16G16B16_UINT = 88, // = VK_FORMAT_R16G16B16_UINT
		R16G16B16_SINT = 89, // = VK_FORMAT_R16G16B16_SINT
		R16G16B16_SFLOAT = 90, // = VK_FORMAT_R16G16B16_SFLOAT
		R16G16B16A16_UINT = 95, // = VK_FORMAT_R16G16B16A16_UINT
		R16G16B16A16_SINT = 96, // = VK_FORMAT_R16G16B16A16_SINT
		R16G16B16A16_SFLOAT = 97, // = VK_FORMAT_R16G16B16A16_SFLOAT
		R32_UINT = 98, // = VK_FORMAT_R32_UINT
		R32_SINT = 99, // = VK_FORMAT_R32_SINT
		R32_SFLOAT = 100, // = VK_FORMAT_R32_SFLOAT
		R32G32_UINT = 101, // = VK_FORMAT_R32G32_UINT
		R32G32_SINT = 102, // = VK_FORMAT_R32G32_SINT
		R32G32_SFLOAT = 103, // = VK_FORMAT_R32G32_SFLOAT
		R32G32B32_UINT = 104, // = VK_FORMAT_R32G32B32_UINT
		R32G32B32_SINT = 105, // = VK_FORMAT_R32G32B32_SINT
		R32G32B32_SFLOAT = 106, // = VK_FORMAT_R32G32B32_SFLOAT
		R32G32B32A32_UINT = 107, // = VK_FORMAT_R32G32B32A32_UINT
		R32G32B32A32_SINT = 108, // = VK_FORMAT_R32G32B32A32_SINT
		R32G32B32A32_SFLOAT = 109, // = VK_FORMAT_R32G32B32A32_SFLOAT
		R64_UINT = 110, // = VK_FORMAT_R64_UINT
		R64_SINT = 111, // = VK_FORMAT_R64_SINT
		R64_SFLOAT = 112, // = VK_FORMAT_R64_SFLOAT
		R64G64_UINT = 113, // = VK_FORMAT_R64G64_UINT
		R64G64_SINT = 114, // = VK_FORMAT_R64G64_SINT
		R64G64_SFLOAT = 115, // = VK_FORMAT_R64G64_SFLOAT
		R64G64B64_UINT = 116, // = VK_FORMAT_R64G64B64_UINT
		R64G64B64_SINT = 117, // = VK_FORMAT_R64G64B64_SINT
		R64G64B64_SFLOAT = 118, // = VK_FORMAT_R64G64B64_SFLOAT
		R64G64B64A64_UINT = 119, // = VK_FORMAT_R64G64B64A64_UINT
		R64G64B64A64_SINT = 120, // = VK_FORMAT_R64G64B64A64_SINT
		R64G64B64A64_SFLOAT = 121, // = VK_FORMAT_R64G64B64A64_SFLOAT
		MaxEnum };
	enum class SPIRVImageType {
		Dim1D = 0,
		Dim2D = 1,
		Dim3D = 2,
		DimCube = 3,
		DimRect = 4,
		DimBuffer = 5,
		DimSubpassData = 6,
		DimTileImageDataEXT = 4173,
		MaxEnum = 0x7fffffff,
	};
	enum class SPIRVResourceType { Undefined = 0, Sampler = 1, CBV = 2, SRC = 4, UAV = 8, PUSH = 16, MaxEnum };
	enum class SPIRVDescriptorType {
		Sampler = 0,        // = VK_DESCRIPTOR_TYPE_SAMPLER
		CombinedImageSampler = 1,        // = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		SampledImage = 2,        // = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
		StorageImage = 3,        // = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		UniformTexelBuffer = 4,        // = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
		StorageTexelBuffer = 5,        // = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
		UniformBuffer = 6,        // = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
		StorageBuffer = 7,        // = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		UniformBufferDynamic = 8,        // = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
		StorageBufferDynamic = 9,        // = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
		InputAttachment = 10,        // = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
		AccerlerationStructure = 1000150000, // = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR
		MaxEnum = 1000150001
	};

	struct SPIRVBlock {
		std::string name;
		uint32_t offset;
		uint32_t size;
		uint32_t paddedSize;
		std::vector<SPIRVBlock> members;
		SPIRVBlock() {
			offset = size = paddedSize = 0;
		}
	};

	struct SPIRVImage {
		SPIRVImageFormat format;
		SPIRVImageType type;
		uint32_t depth;
		uint32_t sampled;
		SPIRVImage() {
			type = SPIRVImageType::MaxEnum;
			format = SPIRVImageFormat::MaxEnum;
			depth = sampled = 0;
		}
	};

	struct SPIRVBinding {
		std::string name;
		SPIRVResourceType restype;
		uint32_t binding;
		uint32_t set;
		uint32_t count;
		SPIRVDescriptorType descriptorType;
		SPIRVShaderStage stageFlags;
		SPIRVImage image;
		SPIRVBlock block;
		SPIRVBinding() {
			restype = SPIRVResourceType::Undefined;
			binding = set = count = UINT32_MAX;
			descriptorType = SPIRVDescriptorType::MaxEnum;
			stageFlags = SPIRVShaderStage::Undefined;
		}
		uint32_t getPaddedSize() {
			uint32_t size = 0;
			for (auto& member : block.members) {
				size += member.paddedSize;
			}
			return size;
		}
	};
	struct SPIRVPushBlock {
		std::string name;
		SPIRVShaderStage stageFlags;
		uint32_t size;
		SPIRVBlock block;
		SPIRVPushBlock() {
			size = 0;
			stageFlags = SPIRVShaderStage::Undefined;
		}
	};

	struct SPIRVInput {
		std::string name;
		SPIRVFormat format;
		SPIRVBlock block;
	};
}