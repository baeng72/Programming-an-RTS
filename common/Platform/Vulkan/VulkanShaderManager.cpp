#pragma once
#include <glm/glm.hpp>
#include "VulkanMeshShader.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "VulkanShaderManager.h"
Renderer::ShaderManager* Renderer::ShaderManager::Create(Renderer::RenderDevice* pdevice) {
	return new Vulkan::VulkanShaderManager(pdevice);
}
namespace Vulkan {
	
	VulkanShaderManager::VulkanShaderManager(Renderer::RenderDevice*pdevice)
		:_pdevice(pdevice) {
		CompileShaders();
	}
	VulkanShaderManager::~VulkanShaderManager() {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		for (auto& shader : _shaderList) {			
			vkDestroyPipelineLayout(context.device, shader.pipelineLayout, nullptr);
			vkDestroyPipeline(context.device, shader.pipeline, nullptr);
		}
		for (auto& shader : _wireframeShaderList) {
			//vkDestroyPipelineLayout(context.device, shader.pipelineLayout, nullptr);
			vkDestroyPipeline(context.device, shader.pipeline, nullptr);
		}
	}
	void* VulkanShaderManager::GetShaderData(Renderer::ShaderManager::ShaderType type,bool wireframe)
	{
		assert((size_t)type < _shaderList.size());
		if (wireframe) {
			return (void*)&_wireframeShaderList[(int)type];
		}
		return (void*)&_shaderList[(int)type];
	}
	void VulkanShaderManager::CompileShaders()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		{

			//Flat shaded
			const char* vertexSrcFlat = R"(
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec4 outColor;

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;
};

layout (push_constant) uniform PushConst{
	mat4 model;
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outColor = inColor;
}
)";
			const char* fragmentSrcFlat = R"(
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec4 inColor;

layout(location=0) out vec4 outFragColor;

void main(){
	outFragColor = inColor;
}
)";

			ShaderCompiler compiler;
			std::vector<uint32_t> vertexSpirv;
			std::vector<uint32_t> fragmentSpirv;
			vertexSpirv = compiler.compileShader(vertexSrcFlat, VK_SHADER_STAGE_VERTEX_BIT);
			fragmentSpirv = compiler.compileShader(fragmentSrcFlat, VK_SHADER_STAGE_FRAGMENT_BIT);
			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorSet descriptorSet;
			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.build(descriptorSet, descriptorSetLayout);
			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
			VkPipelineLayout pipelineLayout;
			PipelineLayoutBuilder::begin(context.device)
				.AddDescriptorSetLayout(descriptorSetLayout)
				.AddPushConstants(pushConstants)
				.build(pipelineLayout);


			std::vector<ShaderModule> shaders;
			VkVertexInputBindingDescription vertexInputDescription = {};
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
			ShaderProgramLoader::begin(context.device)
				.AddShaderSpirv(vertexSpirv)
				.AddShaderSpirv(fragmentSpirv)
				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
			VkPipeline pipeline;
			{
				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(VK_TRUE)
					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
					.build(pipeline);
				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
				_shaderList.push_back(data);
			}
			{
				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(VK_TRUE)
					.setPolygonMode(VK_POLYGON_MODE_LINE)
					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
					.build(pipeline);
				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
				_wireframeShaderList.push_back(data);
			}
			for (auto& shader : shaders) {
				cleanupShaderModule(context.device, shader.shaderModule);
			}
			
		}
		{
			//Flat directional shaded
			const char* vertexSrcFlatDir = R"(
#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;

layout(location=0) out vec3 outNormal;
layout(location=1) out vec4 outColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
	DirectionalLight light;
};

layout (push_constant) uniform PushConst{	
	mat4 model;		//varies per object
};

void main(){
	gl_Position = viewProj * model * vec4(inPos,1.0);
	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
	outColor = inColor;
}
)";
			const char* fragmentSrcFlatDir = R"(
#version 450
layout(location=0) in vec3 inNormal;
layout(location=1) in vec4 inColor;

layout(location=0) out vec4 outFragColor;

struct DirectionalLight{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec3 direction;
};

layout(set=0,binding=0) uniform UBO{
	mat4 viewProj;	//same whole scene
	DirectionalLight light;
};


void main(){
	//normalize interpolated normal
	vec3 normal = normalize(inNormal);
	float shade = max(dot(normal,-light.direction),0);
	vec3 ambient = vec3(inColor)*vec3(light.ambient);
	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
	
	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
	vec4 color = vec4(ambient + diffuse + spec,1.0);
	outFragColor = color;	
}
)";
			ShaderCompiler compiler;
			std::vector<uint32_t> vertexSpirv;
			std::vector<uint32_t> fragmentSpirv;
			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };

			VkDescriptorSetLayout descriptorSetLayout;
			VkDescriptorSet descriptorSet;
			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
				.build(descriptorSet, descriptorSetLayout);

			VkPipelineLayout pipelineLayout;
			PipelineLayoutBuilder::begin(context.device)
				.AddPushConstants(pushConstants)
				.AddDescriptorSetLayout(descriptorSetLayout)
				.build(pipelineLayout);


			std::vector<ShaderModule> shaders;
			VkVertexInputBindingDescription vertexInputDescription = {};
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
			ShaderProgramLoader::begin(context.device)
				.AddShaderSpirv(vertexSpirv)
				.AddShaderSpirv(fragmentSpirv)
				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
			VkPipeline pipeline;
			{
				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(VK_TRUE)
					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
					.build(pipeline);
				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
				_shaderList.push_back(data);
			}
			{
				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(VK_TRUE)
					.setPolygonMode(VK_POLYGON_MODE_LINE)
					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
					.build(pipeline);
				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
				_wireframeShaderList.push_back(data);
			}
			for (auto& shader : shaders) {
				cleanupShaderModule(context.device, shader.shaderModule);
			}
		}
		//build ubos
		{
		UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true)
			.AddBuffer(sizeof(Renderer::FlatShaderUBO), 1, 1)
			.AddBuffer(sizeof(Renderer::FlatShaderDirectionalUBO), 1, 1)
			.build(_uniformBuffer, _uboInfo);
			VkDeviceSize offset = 0;
			for (int i = 0; i < (int)Renderer::ShaderManager::ShaderType::MAX_SHADERS; i++) {
				_shaderList[i].ubo = _uboInfo[i].ptr;				
				_wireframeShaderList[i].ubo = _uboInfo[i].ptr;
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = _uniformBuffer.buffer;
				VkDeviceSize size = _uboInfo[i].objectCount * _uboInfo[i].objectSize * _uboInfo[i].repeatCount;
				bufferInfo.range = size;
				bufferInfo.offset = offset;
				offset += size;
				DescriptorSetUpdater::begin(context.pLayoutCache, _shaderList[i].descriptorSetLayout, _shaderList[i].descriptorSet)
					.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo)
					.update();
			}
		}
	}
}