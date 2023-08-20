#pragma once
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
	/*void* VulkanShaderManager::GetShaderAttribute(Renderer::ShaderAttrData& data)
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		std::tuple<uint32_t, uint32_t, uint32_t> key = std::make_tuple(data.flags, data.stages, data.count);
		if (_shaderAttrMap.find(key) == _shaderAttrMap.end()) {
			VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache);
			uint32_t index = 0;
			if (data.flags & Renderer::ShaderAttrFlagBits::SHADER_ATTR_UBO)
				builder.AddBinding(index++, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, data.stages);
			if (data.flags & Renderer::ShaderAttrFlagBits::SHADER_ATTR_STORAGE)
				builder.AddBinding(index++, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, data.stages);
			if (data.flags & Renderer::ShaderAttrFlagBits::SHADER_ATTR_SAMPLER)
				builder.AddBinding(index++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, data.stages);
			if(data.flags & Renderer::ShaderAttrFlagBits::SHADER_ATTR_SAMPLER_ARRAY)
				builder.AddBinding(index++, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, data.stages, data.count);
			builder.build(descriptorSet, descriptorSetLayout);
			VulkanDescriptorData descData = { descriptorSetLayout,descriptorSet };
			_shaderAttrMap[key] = descData;
		}
		return &_shaderAttrMap[key];
	}*/
	void VulkanShaderManager::CompileShaders()
	{
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
//		{
//
//			//Flat shaded
//			const char* vertexSrcFlat = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//layout(location=2) in vec4 inColor;
//
//layout(location=0) out vec3 outNormal;
//layout(location=1) out vec4 outColor;
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;
//};
//
//layout (push_constant) uniform PushConst{
//	mat4 model;
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	outColor = inColor;
//}
//)";
//			const char* fragmentSrcFlat = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//layout(location=1) in vec4 inColor;
//
//layout(location=0) out vec4 outFragColor;
//
//void main(){
//	outFragColor = inColor;
//}
//)";
//
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlat, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlat, VK_SHADER_STAGE_FRAGMENT_BIT);
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.AddPushConstants(pushConstants)
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//
//		}
//		{
//			//Flat directional shaded
//			const char* vertexSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//layout(location=2) in vec4 inColor;
//
//layout(location=0) out vec3 outNormal;
//layout(location=1) out vec4 outColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//layout (push_constant) uniform PushConst{	
//	mat4 model;		//varies per object
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	outColor = inColor;
//}
//)";
//			const char* fragmentSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//layout(location=1) in vec4 inColor;
//
//layout(location=0) out vec4 outFragColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//
//void main(){
//	//normalize interpolated normal
//	vec3 normal = normalize(inNormal);
//	float shade = max(dot(normal,-light.direction),0);
//	vec3 ambient = vec3(inColor)*vec3(light.ambient);
//	vec3 diffuse = vec3(inColor)*vec3(light.diffuse) * shade;
//	
//	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
//	vec3 spec = vec3(inColor) * vec3(light.specular) * specfactor ;
//	vec4 color = vec4(ambient + diffuse + spec,1.0);
//	outFragColor = color;	
//}
//)";
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddPushConstants(pushConstants)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//		}
//		{
//			//Directional diffuse color material
//			const char* vertexSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//
//
//layout(location=0) out vec3 outNormal;
//
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//
//layout (push_constant) uniform PushConst{	
//	mat4 model;		//varies per object
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	
//}
//)";
//			const char* fragmentSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//
//
//layout(location=0) out vec4 outFragColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//layout(set=1,binding=0) uniform MAT{
//	vec4 color;
//};
//
//
//
//void main(){
//	//normalize interpolated normal
//	vec3 normal = normalize(inNormal);
//	float shade = max(dot(normal,-light.direction),0);
//	vec3 ambient = vec3(color)*vec3(light.ambient);
//	vec3 diffuse = vec3(color)*vec3(light.diffuse) * shade;
//	
//	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
//	vec3 spec = vec3(color) * vec3(light.specular) * specfactor ;
//	vec4 finalColor = vec4(ambient + diffuse + spec,color.w);
//	outFragColor = finalColor;	
//}
//)";
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddPushConstants(pushConstants)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.AddDescriptorSetLayout(descriptorSetLayout)//repeat for set 1 uniform 
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//		}
//		{
//			//Directional diffuse texture material
//			const char* vertexSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//layout(location=2) in vec2 inUV;
//
//layout(location=0) out vec3 outNormal;
//layout(location=1) out vec2 outUV;
//
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//
//layout (push_constant) uniform PushConst{	
//	mat4 model;		//varies per object
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	outUV = inUV;
//}
//)";
//			const char* fragmentSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//layout(location=1) in vec2 inUV;
//
//layout(location=0) out vec4 outFragColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//layout(set=1,binding=0) uniform sampler2D diffuseTex;
//	
//
//
//
//void main(){
//	//normalize interpolated normal
//	vec3 normal = normalize(inNormal);
//	vec4 color = texture(diffuseTex,inUV);
//	float shade = max(dot(normal,-light.direction),0);
//	vec3 ambient = vec3(color)*vec3(light.ambient);
//	vec3 diffuse = vec3(color)*vec3(light.diffuse) * shade;
//	
//	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
//	vec3 spec = vec3(color) * vec3(light.specular) * specfactor ;
//	vec4 finalColor = vec4(ambient + diffuse + spec,color.w);
//	outFragColor = finalColor;	
//}
//)";
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddPushConstants(pushConstants)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//
//		}
//		{
//			//Directional diffuse array
//			const char* vertexSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//
//
//layout(location=0) out vec3 outNormal;
//
//
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//
//layout (push_constant) uniform PushConst{	
//	mat4 model;		//varies per object
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	
//}
//)";
//			const char* fragmentSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//
//
//layout(location=0) out vec4 outFragColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//layout(set=1,binding=0) readonly storage colorArray{
//	vec4 attrs[];
//}colors;
//	
//
//
//
//void main(){
//	//normalize interpolated normal
//	vec3 normal = normalize(inNormal);
//	vec4 color = colors.attrs[gl_PrimitiveIndex];
//	float shade = max(dot(normal,-light.direction),0);
//	vec3 ambient = vec3(color)*vec3(light.ambient);
//	vec3 diffuse = vec3(color)*vec3(light.diffuse) * shade;
//	
//	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
//	vec3 spec = vec3(color) * vec3(light.specular) * specfactor ;
//	vec4 finalColor = vec4(ambient + diffuse + spec,color.w);
//	outFragColor = finalColor;	
//}
//)";
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddPushConstants(pushConstants)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//			
//		}
//		{
//			//Directional diffuse texture material
//			const char* vertexSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inPos;
//layout(location=1) in vec3 inNormal;
//layout(location=2) in vec2 inUV;
//
//layout(location=0) out vec3 outNormal;
//layout(location=1) out vec2 outUV;
//
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//
//layout (push_constant) uniform PushConst{	
//	mat4 model;		//varies per object
//};
//
//void main(){
//	gl_Position = viewProj * model * vec4(inPos,1.0);
//	outNormal = vec3(inverse(model) * vec4(inNormal,0.0));
//	outUV = inUV;
//}
//)";
//			const char* fragmentSrcFlatDir = R"(
//#version 450
//layout(location=0) in vec3 inNormal;
//layout(location=1) in vec2 inUV;
//
//layout(location=0) out vec4 outFragColor;
//
//struct DirectionalLight{
//	vec4 diffuse;
//	vec4 ambient;
//	vec4 specular;
//	vec3 direction;
//};
//
//layout(set=0,binding=0) uniform UBO{
//	mat4 viewProj;	//same whole scene
//	DirectionalLight light;
//};
//
//layout(set=1,binding=0) readonly storage matAttr{
//	int attrs[];
//}material;
//layout(set=1,binding=1) uniform sampler2D diffuseTextures[];//may need to put a value in this, with dummy texture filling empty spaces?
//	
//
//
//
//void main(){
//	//normalize interpolated normal
//	vec3 normal = normalize(inNormal);
//	int attr = material.attrs[gl_PrimitiveIndex];
//	vec4 color = texture(diffuseTextures[attr],inUV);
//	float shade = max(dot(normal,-light.direction),0);
//	vec3 ambient = vec3(color)*vec3(light.ambient);
//	vec3 diffuse = vec3(color)*vec3(light.diffuse) * shade;
//	
//	float specfactor = pow(shade,32);//hack specular (no view dir at moment)
//	vec3 spec = vec3(color) * vec3(light.specular) * specfactor ;
//	vec4 finalColor = vec4(ambient + diffuse + spec,color.w);
//	outFragColor = finalColor;	
//}
//)";
//			ShaderCompiler compiler;
//			std::vector<uint32_t> vertexSpirv;
//			std::vector<uint32_t> fragmentSpirv;
//			vertexSpirv = compiler.compileShader(vertexSrcFlatDir, VK_SHADER_STAGE_VERTEX_BIT);
//			fragmentSpirv = compiler.compileShader(fragmentSrcFlatDir, VK_SHADER_STAGE_FRAGMENT_BIT);
//			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(Renderer::FlatShaderPushConst)} };
//
//			VkDescriptorSetLayout descriptorSetLayout;
//			VkDescriptorSet descriptorSet;
//			DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
//				.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
//				.build(descriptorSet, descriptorSetLayout);
//
//			VkPipelineLayout pipelineLayout;
//			PipelineLayoutBuilder::begin(context.device)
//				.AddPushConstants(pushConstants)
//				.AddDescriptorSetLayout(descriptorSetLayout)
//				.build(pipelineLayout);
//
//
//			std::vector<ShaderModule> shaders;
//			VkVertexInputBindingDescription vertexInputDescription = {};
//			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
//			ShaderProgramLoader::begin(context.device)
//				.AddShaderSpirv(vertexSpirv)
//				.AddShaderSpirv(fragmentSpirv)
//				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
//			VkPipeline pipeline;
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					//.setCullMode(VK_CULL_MODE_FRONT_BIT)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_shaderList.push_back(data);
//			}
//			{
//				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
//					.setBlend(VK_TRUE)
//					.setPolygonMode(VK_POLYGON_MODE_LINE)
//					.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
//					.build(pipeline);
//				VulkanShaderData data{ descriptorSetLayout,descriptorSet,pipelineLayout,pipeline };
//				_wireframeShaderList.push_back(data);
//			}
//			for (auto& shader : shaders) {
//				cleanupShaderModule(context.device, shader.shaderModule);
//			}
//			
//			
//
//		}
//		//build ubos
//		{
//			UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true)
//				.AddBuffer(sizeof(Renderer::FlatShaderUBO), 1, 1)
//				.AddBuffer(sizeof(Renderer::FlatShaderDirectionalUBO), 1, 1)
//				.build(_uniformBuffer, _uboInfo);
//			VkDeviceSize offset = 0;
//			for (int i = 0; i < (int)Renderer::ShaderManager::ShaderType::MAX_SHADERS; i++) {
//				_shaderList[i].ubo = _uboInfo[i].ptr;
//				_wireframeShaderList[i].ubo = _uboInfo[i].ptr;
//				VkDescriptorBufferInfo bufferInfo{};
//				bufferInfo.buffer = _uniformBuffer.buffer;
//				VkDeviceSize size = _uboInfo[i].objectCount * _uboInfo[i].objectSize * _uboInfo[i].repeatCount;
//				bufferInfo.range = size;
//				bufferInfo.offset = offset;
//				offset += size;
//				DescriptorSetUpdater::begin(context.pLayoutCache, _shaderList[i].descriptorSetLayout, _shaderList[i].descriptorSet)
//					.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo)
//					.update();
//			}
//		}
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
	void VulkanShaderManager::CompileShader(const std::string&name,const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources,bool cullBackFaces, bool enableBlend)
	{
		ShaderCompiler compiler;
		std::unordered_map<VkShaderStageFlagBits, std::vector<uint32_t>> spirvMap;
		for (auto &pair : shaderSources) {
			std::vector<uint32_t> spirv = compiler.compileShader(pair.second.c_str(),pair.first);
			spirvMap[pair.first] = spirv;
		}
		//get descriptor sets, ubo sizes, etc
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<VkDescriptorSetLayoutBinding>>> shaderBindings;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<std::string>>> shaderBindingNames;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::vector<uint32_t>>> shaderBindingSizes;
		std::unordered_map < VkShaderStageFlagBits, std::vector<std::tuple<std::string, VkFormat, uint32_t>>> shaderInputs;
		std::unordered_map<VkShaderStageFlagBits, std::vector<VkPushConstantRange>> pushConstRanges;
		std::unordered_map<VkShaderStageFlagBits, std::vector<std::string>> pushConstNames;
		uint32_t maxSet = 0;
		uint32_t maxPushConst = 0;
		for (auto& pair : spirvMap) {
			auto& spirv = pair.second;
			SpvReflectShaderModule module = {};
			SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);
			ASSERT(result == SPV_REFLECT_RESULT_SUCCESS,"Unable to reflect shader spirv!");


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
			ASSERT(stage == pair.first, "Invalid shader stage reflecting spirv!");
			uint32_t descriptorSetCount = module.descriptor_set_count;
			std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings(descriptorSetCount);
			std::vector<std::vector<std::string>> descriptorSetLayoutBindingNames(descriptorSetCount);
			std::vector<std::vector<uint32_t>> descriptorSetLayoutBindingSizes(descriptorSetCount);
			for (uint32_t i = 0; i < descriptorSetCount; i++) {
				SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
				maxSet = std::max(srcset.set+1, maxSet);
				auto& dstset = descriptorSetLayoutBindings[srcset.set];
				auto& dstsetnames = descriptorSetLayoutBindingNames[srcset.set];
				auto& dstsetsizes = descriptorSetLayoutBindingSizes[srcset.set];
				uint32_t descriptorSetBindingCount = srcset.binding_count;
				dstset.resize(descriptorSetBindingCount);
				dstsetnames.resize(descriptorSetBindingCount);
				dstsetsizes.resize(descriptorSetBindingCount);
				for (uint32_t j = 0; j < descriptorSetBindingCount; j++) {
					SpvReflectDescriptorBinding& srcbinding = *srcset.bindings[j];
					dstset[j].binding = srcbinding.binding;
					dstset[j].descriptorType = (VkDescriptorType)srcbinding.descriptor_type;
					dstset[j].stageFlags = stage;
					dstset[j].descriptorCount = srcbinding.count;
					dstset[j].pImmutableSamplers = nullptr;
					dstsetnames[j] = dstset[j].descriptorType ==  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? srcbinding.name : srcbinding.type_description->type_name;
					uint32_t typeFlags = srcbinding.type_description->type_flags;
					switch (dstset[j].descriptorType) {
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
					case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
						uint32_t size = 0;
						if (typeFlags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {
							size = srcbinding.block.padded_size;
							dstsetsizes[j] = size;
						}
						std::vector<std::string> names(srcbinding.type_description->member_count);
						std::vector<uint32_t> strides(srcbinding.type_description->member_count);
						for (uint32_t m = 0; m < srcbinding.type_description->member_count; m++) {
							uint32_t mstride = 0;
							SpvReflectTypeDescription& member = srcbinding.type_description->members[m];
							names[m] = member.struct_member_name;
							mstride = GetMemberSize(member);
							/*if (member.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
							mstride = member.traits.numeric.matrix.row_count * member.traits.numeric.matrix.column_count * (member.traits.numeric.scalar.width >> 4);
							}
							else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
							mstride = member.traits.numeric.vector.component_count * (member.traits.numeric.scalar.width >> 4);
							}
							else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
							mstride = member.traits.numeric.scalar.width >> 4;
							}
							else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
							mstride = member.traits.numeric.scalar.width >> 4;
							}
							else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) {
								for (uint32_t m2 = 0; m2 < member.member_count; m2++) {
									SpvReflectTypeDescription& member2 = member.members[m2];

								}
							}*/
							strides[m] = mstride;
							//size += mstride;
						}
						
					}
						break;
					case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
						break;
					}
				}
			}
			shaderBindings[stage] = descriptorSetLayoutBindings;
			shaderBindingNames[stage] = descriptorSetLayoutBindingNames;
			shaderBindingSizes[stage] = descriptorSetLayoutBindingSizes;
			uint32_t offset = 0;
			std::vector<uint32_t> sizes(module.input_variable_count);
			std::vector<VkFormat> formats(module.input_variable_count);
			std::vector<std::string> names(module.input_variable_count);
			std::vector<std::tuple<std::string, VkFormat, uint32_t>> inputs(module.input_variable_count);
			for (uint32_t i = 0; i < module.input_variable_count; ++i) {
				SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];				
				uint32_t size = 0;
				/*uint32_t flags = inputVar.type_description->type_flags;
				if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
					size = inputVar.numeric.vector.component_count * (inputVar.numeric.scalar.width >> 3);
				}*/
				if (inputVar.location > module.input_variable_count && inputVar.built_in)
					continue;//won't happen in vertex shader?
				names[inputVar.location] = inputVar.name;
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
				default:
					assert(0);
					break;
				}
				sizes[inputVar.location] = (uint32_t)size;
				formats[inputVar.location] = (VkFormat)inputVar.format;
				inputs[inputVar.location] = std::tuple<std::string, VkFormat, uint32_t>(inputVar.name, (VkFormat)inputVar.format, size);
			}
			shaderInputs[stage] = inputs;
			//push constants
			uint32_t pushConstCount = module.push_constant_block_count;
			maxPushConst = std::max(pushConstCount, maxPushConst);
			std::vector<VkPushConstantRange> pushConstRange(pushConstCount);
			std::vector<std::string> pushConstName(pushConstCount);
			for (uint32_t p = 0; p < pushConstCount; p++) {
				SpvReflectBlockVariable & pc = module.push_constant_blocks[p];
				uint32_t typeFlags = pc.type_description->type_flags;
				uint32_t size = 0;
				if (typeFlags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {
					size = pc.padded_size;
					
				}
				else {
					assert(0);
				}
				pushConstRange[p].offset = 0;
				pushConstRange[p].size = size;
				pushConstRange[p].stageFlags = stage;
				pushConstName[p] = pc.type_description->type_name;
			}
			pushConstRanges[stage] = pushConstRange;
			pushConstNames[stage] = pushConstName;
			spvReflectDestroyShaderModule(&module);
		}
		//merge bindings
		std::vector<std::vector<VkDescriptorSetLayoutBinding>> mergedBindings(maxSet);
		std::vector<std::vector<std::string>> mergedNames(maxSet);
		std::vector<std::vector<uint32_t>> mergedSizes(maxSet);
		for (auto& pair : shaderBindings) {
			VkShaderStageFlagBits stage = pair.first;
			auto& setnames = shaderBindingNames[stage];
			auto& setsizes = shaderBindingSizes[stage];
			auto setbindings = pair.second;
			for (size_t i = 0; i < setbindings.size();i++) {
				auto& srcset = setbindings[i];	
				auto& srcnames = setnames[i];
				auto& srcsizes = setsizes[i];
				auto& dstset = mergedBindings[i];
				if (dstset.size() < srcset.size())
					dstset.resize(srcset.size());
				auto& dstnames = mergedNames[i];
				if (dstnames.size() < srcnames.size())
					dstnames.resize(srcnames.size());
				auto& dstsizes = mergedSizes[i];
				if (dstsizes.size() < srcsizes.size())
					dstsizes.resize(srcsizes.size());
				for (size_t j = 0; j < srcset.size(); j++) {
					dstset[j].binding = srcset[j].binding;
					dstset[j].descriptorCount = srcset[j].descriptorCount;
					dstset[j].descriptorType = srcset[j].descriptorType;
					dstset[j].pImmutableSamplers = srcset[j].pImmutableSamplers;
					dstset[j].stageFlags |= stage;
					ASSERT(dstnames[j].empty() || dstnames[j] == srcnames[j], "Incorrect set binding name!");
					dstnames[j] = srcnames[j];
					dstsizes[j] = srcsizes[j];
				}
				
			}
		}
		//merge push constants
		std::vector<VkPushConstantRange> mergedPushConsts(maxPushConst);
		for (auto& pair : pushConstRanges) {
			VkShaderStageFlagBits stage = pair.first;
			auto srcranges = pushConstRanges[pair.first];
			for(size_t i=0;i<srcranges.size();i++){
				auto& srcrange = srcranges[i];
				auto& dstrange = mergedPushConsts[i];
				dstrange.offset = srcrange.offset;
				dstrange.size = srcrange.size;
				dstrange.stageFlags |= stage;

			}
			
		}
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& framedata = *framedataptr;
		//build descriptor sets
		std::vector<VkDescriptorSetLayout> layouts;	
		for (auto& set : mergedBindings) {
			auto layoutbuilder = DescriptorSetLayoutBuilder::begin(context.pLayoutCache);
			for (auto& binding : set) {
				layoutbuilder.AddBinding(binding);
			}			
			VkDescriptorSetLayout descriptorLayout = layoutbuilder.build();
			layouts.push_back(descriptorLayout);
			
		}
		VkPipelineLayout pipelineLayout;
		PipelineLayoutBuilder::begin(context.device)
			.AddDescriptorSetLayouts(layouts)
			.AddPushConstants(mergedPushConsts)
			.build(pipelineLayout);

		std::vector<ShaderModule> shaders;
		for (auto& pair : spirvMap) {
			auto& spirv = pair.second;
			VkShaderModule shader = VK_NULL_HANDLE;

			VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			createInfo.codeSize = spirv.size() * sizeof(uint32_t);
			createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
			VkResult res = vkCreateShaderModule(context.device, &createInfo, nullptr, &shader);
			ASSERT(res == VK_SUCCESS,"Unable to create shader module!");
			shaders.push_back({ shader,pair.first });
		}
		VkVertexInputBindingDescription vertexInputDescription;
		
		auto& vertexInputs = shaderInputs[VK_SHADER_STAGE_VERTEX_BIT];
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions(vertexInputs.size());
		uint32_t offset = 0;
		for(size_t i=0;i<vertexInputs.size();i++){
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
			PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
				.setBlend(enableBlend?VK_TRUE:VK_FALSE)
				.setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
				.setCullMode(cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT)
				.setDepthTest(VK_TRUE)//need this to be a parameter
				.build(pipeline);
			VulkanShaderData shaderData;
			shaderData.descriptorSetLayouts = layouts;
			
			shaderData.pipeline = shaderData.filledPipeline = pipeline;
			shaderData.pipelineLayout = pipelineLayout;
			_shaderList[name] = shaderData;
		}
		{
			VkPipeline pipeline = VK_NULL_HANDLE;
			//wireframe
			PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
				.setPolygonMode(VK_POLYGON_MODE_LINE)
				.setCullMode(cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT)
				.setDepthTest(VK_TRUE)//need this to be a parameter
				.build(pipeline);
			_shaderList[name].wireframePipeline = pipeline;
			
			
		}
		for (auto& shader : shaders) {
			cleanupShaderModule(context.device, shader.shaderModule);
		}
		//create uniforms, if any
		std::vector<VkDeviceSize> uniformSizes;
		std::vector<std::string> uniformNames;
		std::vector<uint32_t> uboSetBindings;
		std::vector<VkDeviceSize> storageSizes;
		std::vector<std::string> storageNames;
		std::vector<uint32_t> storageSetBindings;
		std::vector<std::string> imageNames;
		std::vector<uint32_t> imageSetBindings;
		std::vector<uint32_t> imageCounts;
		for (uint32_t s = 0; s < mergedBindings.size(); s++) {
			auto& set = mergedBindings[s];
			for (uint32_t b = 0; b < set.size(); b++) {
				auto& binding = set[b];
				uint32_t setBinding = s << 16 | b;
				if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
					uniformSizes.push_back(mergedSizes[s][b]);
					uniformNames.push_back(mergedNames[s][b]);					
					uboSetBindings.push_back(setBinding);
				}
				else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
					storageSizes.push_back(mergedSizes[s][b]);
					storageNames.push_back(mergedNames[s][b]);					
					storageSetBindings.push_back(setBinding);
				}
				else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
					imageNames.push_back(mergedNames[s][b]);
					imageSetBindings.push_back(setBinding);
					imageCounts.push_back(binding.descriptorCount);
				}
			}
		}
		//allocate uniforms, can't do with storage buffers as size is determined in app, similar with dynamic buffers
		if (uniformSizes.size() > 0) {
			std::vector<UniformBufferInfo> uboLayoutInfo;
			Vulkan::Buffer uniformBuffer;
			auto unibuilder = UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true);
			for (auto& size : uniformSizes) {
				unibuilder.AddBuffer(size, 1, 1);
			}
			unibuilder.build(uniformBuffer, uboLayoutInfo);
			
			
			std::unordered_map<std::string, void*> uboMap;
			std::unordered_map<std::string, VkDeviceSize> uboSizeMap;
			for (size_t i = 0; i < uniformNames.size();i++) {
				uboMap[uniformNames[i]] = uboLayoutInfo[i].ptr;
				uboSizeMap[uniformNames[i]] = uniformSizes[i];
			}
			_shaderList[name].uboMap = uboMap;
			_shaderList[name].uboSizeMap = uboSizeMap;			
			_shaderList[name].uboNames = uniformNames;
			_shaderList[name].uboSetBindings = uboSetBindings;
			_shaderList[name].uniformBuffer = uniformBuffer;
			_shaderList[name].imageNames = imageNames;
			_shaderList[name].imageSetBindings = imageSetBindings;
			_shaderList[name].imageCounts = imageCounts;
			/*uint32_t index = 0;
			VkDeviceSize offset = 0;
			
			for (auto&uboBinding:uboSetBindings) {
				uint32_t set = uboBinding >> 16;
				uint32_t binding = uboBinding & 0xFF;
				auto uniupdated = DescriptorSetUpdater::begin(context.pLayoutCache, layouts[set], sets[set]);
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = uniformBuffer.buffer;
				bufferInfo.offset = offset;
				VkDeviceSize size = uboLayoutInfo[index].objectCount * uboLayoutInfo[index].objectSize * uboLayoutInfo[index].repeatCount;
				bufferInfo.range = size;
				index++;
				offset += size;

				uniupdated.AddBinding(binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
					
				uniupdated.update();
			}*/
		}
		_shaderList[name].storageNames = storageNames;
		_shaderList[name].storageSetBindings = storageSetBindings;
		for (auto& name : storageNames) {//dummy values, do we need a map and not just an array?
			_shaderList[name].storageMap[name] = nullptr;
			_shaderList[name].storageSizeMap[name] = 0;
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
	void* VulkanShaderManager::CreateShaderData(const char* shaderPath,bool cullBackFaces,bool enableBlend) {
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::string source = readFile(filepath);
			const std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources = PreProcess(source);
			CompileShader(name,shaderSources,cullBackFaces,enableBlend);
		}
		return &_shaderList[name];
	}
}