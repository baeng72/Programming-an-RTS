
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
	void VulkanShaderManager::CompileShader(const std::string&name,const std::unordered_map<VkShaderStageFlagBits, std::string>& shaderSources,bool cullBackFaces, bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes)
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
			std::vector<VkPushConstantRange> pushConstRanges = { {reflection.pushBlock.stageFlags,0,reflection.pushBlock.size} };

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
				PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
					.setBlend(enableBlend ? VK_TRUE : VK_FALSE)
					.setDepthTest(enableDepth ? VK_TRUE : VK_FALSE)
					.setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
					.setCullMode(cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT)
					.setDepthTest(VK_TRUE)//need this to be a parameter
					.build(pipeline);
				auto& shaderData = _shaderList[name];
				shaderData.descriptorSetLayouts = layouts;

				shaderData.pipeline = shaderData.filledPipeline = pipeline;
				shaderData.pipelineLayout = pipelineLayout;
				
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
							////this is the correct memory location
							//std::get<7>(reflection.blockmembers[j]) = bufferInfo[i].ptr;
							//for (size_t k = j + 1; k < reflection.blockmembers.size(); k++) {
							//	int s3 = std::get<2>(reflection.blockmembers[k]);
							//	int b3 = std::get<3>(reflection.blockmembers[k]);
							//	if(s==s3 && b==b2){
							//	uint32_t offset = std::get<4>(reflection.blockmembers[k]);
							//}
							//break;
						}
					}
				}
			}




		}
		
		//for (auto& pair : spirvMap) {
		//	auto& spirv = pair.second;
		//	SpvReflectShaderModule module = {};
		//	SpvReflectResult result = spvReflectCreateShaderModule(spirv.size() * sizeof(uint32_t), spirv.data(), &module);
		//	ASSERT(result == SPV_REFLECT_RESULT_SUCCESS,"Unable to reflect shader spirv!");


		//	VkShaderStageFlagBits stage;
		//	switch (module.shader_stage) {
		//	case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		//		stage = VK_SHADER_STAGE_VERTEX_BIT;
		//		break;
		//	case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		//		stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		//		break;
		//	case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
		//		stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		//		break;
		//	case SpvReflectShaderStageFlagBits::SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
		//		stage = VK_SHADER_STAGE_COMPUTE_BIT;
		//		break;
		//	default:
		//		assert(0);
		//		break;
		//	}
		//	ASSERT(stage == pair.first, "Invalid shader stage reflecting spirv!");
		//	uint32_t descriptorSetCount = module.descriptor_set_count;
		//	for (uint32_t i = 0; i < descriptorSetCount; i++) {
		//		SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
		//		maxSet = std::max(srcset.set + 1, maxSet);
		//	}
		//	std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutBindings(maxSet);
		//	std::vector<std::vector<std::string>> descriptorSetLayoutBindingNames(maxSet);
		//	std::vector<std::vector<std::string>> descriptorSetLayoutCombinedBindingNames(maxSet);
		//	std::vector<std::vector<uint32_t>> descriptorSetCombinedOffsets(maxSet);
		//	std::vector<std::vector<uint32_t>> descriptorSetLayoutBindingSizes(maxSet);
		//	//uint32_t typeIndex = 0;
		//	for (uint32_t i = 0; i < descriptorSetCount; i++) {
		//		SpvReflectDescriptorSet& srcset = module.descriptor_sets[i];
		//		maxSet = std::max(srcset.set+1, maxSet);
		//		auto& dstset = descriptorSetLayoutBindings[srcset.set];
		//		auto& dstsetnames = descriptorSetLayoutBindingNames[srcset.set];
		//		auto& dstcombinednames = descriptorSetLayoutCombinedBindingNames[srcset.set];
		//		auto& dstcombinedoffsets = descriptorSetCombinedOffsets[srcset.set];
		//		auto& dstsetsizes = descriptorSetLayoutBindingSizes[srcset.set];
		//		uint32_t descriptorSetBindingCount = srcset.binding_count;
		//		dstset.resize(descriptorSetBindingCount);
		//		dstsetnames.resize(descriptorSetBindingCount);
		//		dstsetsizes.resize(descriptorSetBindingCount);
		//		
		//		for (uint32_t j = 0; j < descriptorSetBindingCount; j++) {
		//			SpvReflectDescriptorBinding& srcbinding = *srcset.bindings[j];
		//			dstset[j].binding = srcbinding.binding;
		//			dstset[j].descriptorType = (VkDescriptorType)srcbinding.descriptor_type;
		//			/*if (ptypes) {
		//				if(typeIndex>numtypes)
		//					LOG_WARN("More shader storage types {0} than expected:{1}", typeIndex, numtypes);
		//				switch (dstset[j].descriptorType) {
		//				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		//					if (!(ptypes[typeIndex] != Renderer::ShaderStorageType::Uniform|| ptypes[typeIndex] != Renderer::ShaderStorageType::UniformDynamic))
		//						LOG_WARN("Expected Uniform Type at shader index {0}", typeIndex);
		//					break;
		//				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		//					if (!(ptypes[typeIndex] != Renderer::ShaderStorageType::UniformDynamic|| ptypes[typeIndex] != Renderer::ShaderStorageType::StorageDynamic))
		//						LOG_WARN("Expected Uniform Dynamic Type at shader index {0}", typeIndex);
		//					break;
		//				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		//					if (ptypes[typeIndex] != Renderer::ShaderStorageType::Storage)
		//						LOG_WARN("Expected Storage Buffer Type at shader index {0}", typeIndex);
		//					break;
		//				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		//					if (ptypes[typeIndex] != Renderer::ShaderStorageType::StorageDynamic)
		//						LOG_WARN("Expected Storage Buffer Dynamic Type at shader index {0}", typeIndex);
		//					break;
		//				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		//					if (ptypes[typeIndex] != Renderer::ShaderStorageType::Texture)
		//						LOG_WARN("Expected Texture Type at shader index {0}", typeIndex);
		//					break;
		//				}
		//			}
		//			typeIndex++;*/
		//			dstset[j].stageFlags = stage;
		//			dstset[j].descriptorCount = srcbinding.count;
		//			dstset[j].pImmutableSamplers = nullptr;
		//			dstsetnames[j] = dstset[j].descriptorType ==  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ? srcbinding.name : srcbinding.type_description->type_name;
		//			uint32_t typeFlags = srcbinding.type_description->type_flags;
		//			switch (dstset[j].descriptorType) {
		//			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		//			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		//			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		//			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
		//				uint32_t size = 0;
		//				if (typeFlags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {
		//					size = srcbinding.block.padded_size;
		//					dstsetsizes[j] = size;
		//				}
		//				std::vector<std::string> combinednames(srcbinding.type_description->member_count);
		//				
		//				std::vector<std::string> membernames(srcbinding.type_description->member_count);
		//				std::vector<uint32_t> memberoffsets(srcbinding.type_description->member_count);
		//				std::vector<uint32_t> membersizes(srcbinding.type_description->member_count);
		//				std::vector<uint32_t> memberstrides(srcbinding.type_description->member_count);
		//				dstcombinednames.resize(srcbinding.type_description->member_count);
		//				dstcombinedoffsets.resize(srcbinding.type_description->member_count);
		//				uint32_t memberoffset = 0;
		//				for (uint32_t m = 0; m < srcbinding.block.member_count; m++) {
		//					auto& member = srcbinding.block.members[m];
		//					uint32_t stride = member.padded_size;
		//					uint32_t size = member.size;
		//					uint32_t offset = member.offset;
		//					membernames[m] = member.name;
		//					membersizes[m] = size;
		//					memberoffsets[m] = offset;
		//					memberstrides[m] = stride;
		//					//char buffer[128];
		//					//sprintf_s(buffer, "%s.%s", dstsetnames[j].c_str(), member.name);
		//					combinednames[m] = dstsetnames[j] + "." + member.name;// buffer;
		//					dstcombinednames[m] = dstsetnames[j] + "." + member.name;
		//					dstcombinedoffsets[m] = offset;
		//				}
		//				
		//				//for (uint32_t m = 0; m < srcbinding.type_description->member_count; m++) {
		//				//	uint32_t membersize = 0;
		//				//	SpvReflectTypeDescription& member = srcbinding.type_description->members[m];
		//				//	membernames[m] = member.struct_member_name;
		//				//	membersize = GetMemberSize(member);
		//				//	/*if (member.type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX) {
		//				//	mstride = member.traits.numeric.matrix.row_count * member.traits.numeric.matrix.column_count * (member.traits.numeric.scalar.width >> 4);
		//				//	}
		//				//	else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
		//				//	mstride = member.traits.numeric.vector.component_count * (member.traits.numeric.scalar.width >> 4);
		//				//	}
		//				//	else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
		//				//	mstride = member.traits.numeric.scalar.width >> 4;
		//				//	}
		//				//	else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_INT) {
		//				//	mstride = member.traits.numeric.scalar.width >> 4;
		//				//	}
		//				//	else if (member.type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) {
		//				//		for (uint32_t m2 = 0; m2 < member.member_count; m2++) {
		//				//			SpvReflectTypeDescription& member2 = member.members[m2];

		//				//		}
		//				//	}*/
		//				//	membersizes[m] = membersize;
		//				//	memberoffsets[m] = memberoffset;
		//				//	memberoffset += member.traits.numeric.scalar.width;
		//				//	//size += mstride;
		//				//}
		//				//
		//			}
		//				break;
		//			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		//				break;
		//			}
		//		}
		//	}
		//	shaderBindings[stage] = descriptorSetLayoutBindings;
		//	shaderBindingNames[stage] = descriptorSetLayoutBindingNames;
		//	shaderBindingCombinedNames[stage] = descriptorSetLayoutCombinedBindingNames;
		//	shaderBindingCombinedOffsets[stage] = descriptorSetCombinedOffsets;
		//	shaderBindingSizes[stage] = descriptorSetLayoutBindingSizes;
		//	uint32_t offset = 0;
		//	std::vector<uint32_t> sizes(module.input_variable_count);
		//	std::vector<VkFormat> formats(module.input_variable_count);
		//	std::vector<std::string> names(module.input_variable_count);
		//	std::vector<std::tuple<std::string, VkFormat, uint32_t>> inputs(module.input_variable_count);
		//	for (uint32_t i = 0; i < module.input_variable_count; ++i) {
		//		SpvReflectInterfaceVariable& inputVar = *module.input_variables[i];				
		//		uint32_t size = 0;
		//		/*uint32_t flags = inputVar.type_description->type_flags;
		//		if (flags & SPV_REFLECT_TYPE_FLAG_VECTOR) {
		//			size = inputVar.numeric.vector.component_count * (inputVar.numeric.scalar.width >> 3);
		//		}*/
		//		if (inputVar.location > module.input_variable_count && inputVar.built_in)
		//			continue;//won't happen in vertex shader?
		//		names[inputVar.location] = inputVar.name;
		//		switch (inputVar.format) {
		//		case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
		//			size = sizeof(float) * 4;
		//			break;
		//		case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
		//			size = sizeof(float) * 3;
		//			break;
		//		case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
		//			size = sizeof(float) * 2;
		//			break;
		//		case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
		//			size = sizeof(int) * 4;
		//			break;
		//		default:
		//			assert(0);
		//			break;
		//		}
		//		sizes[inputVar.location] = (uint32_t)size;
		//		formats[inputVar.location] = (VkFormat)inputVar.format;
		//		inputs[inputVar.location] = std::tuple<std::string, VkFormat, uint32_t>(inputVar.name, (VkFormat)inputVar.format, size);
		//	}
		//	shaderInputs[stage] = inputs;
		//	//push constants
		//	uint32_t pushConstCount = module.push_constant_block_count;
		//	maxPushConst = std::max(pushConstCount, maxPushConst);
		//	std::vector<VkPushConstantRange> pushConstRange(pushConstCount);
		//	std::vector<std::string> pushConstName(pushConstCount);
		//	for (uint32_t p = 0; p < pushConstCount; p++) {
		//		SpvReflectBlockVariable & pc = module.push_constant_blocks[p];
		//		uint32_t typeFlags = pc.type_description->type_flags;
		//		uint32_t size = 0;
		//		if (typeFlags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) {
		//			size = pc.padded_size;
		//			
		//		}
		//		else {
		//			assert(0);
		//		}
		//		pushConstRange[p].offset = 0;
		//		pushConstRange[p].size = size;
		//		pushConstRange[p].stageFlags = stage;
		//		pushConstName[p] = pc.type_description->type_name;
		//	}
		//	pushConstRanges[stage] = pushConstRange;
		//	pushConstNames[stage] = pushConstName;
		//	spvReflectDestroyShaderModule(&module);
		//}
		////merge bindings
		//std::vector<std::vector<VkDescriptorSetLayoutBinding>> mergedBindings(maxSet);
		//std::vector<std::vector<std::string>> mergedNames(maxSet);
		//std::vector<std::vector<std::string>> mergedCombinedNames(maxSet);
		//std::vector<std::vector<uint32_t>> mergedSizes(maxSet);
		//std::vector<std::vector<uint32_t>> mergedCombinedOffsets(maxSet);
		//
		//for (auto& pair : shaderBindings) {
		//	
		//	VkShaderStageFlagBits stage = pair.first;
		//	{
		//		auto& setnames = shaderBindingNames[stage];
		//		auto& setsizes = shaderBindingSizes[stage];
		//		auto setbindings = pair.second;
		//		for (size_t i = 0; i < setbindings.size(); i++) {
		//			auto& srcset = setbindings[i];
		//			auto& srcnames = setnames[i];
		//			auto& srcsizes = setsizes[i];
		//			auto& dstset = mergedBindings[i];
		//			if (dstset.size() < srcset.size())
		//				dstset.resize(srcset.size());
		//			auto& dstnames = mergedNames[i];
		//			if (dstnames.size() < srcnames.size())
		//				dstnames.resize(srcnames.size());
		//			auto& dstsizes = mergedSizes[i];
		//			if (dstsizes.size() < srcsizes.size())
		//				dstsizes.resize(srcsizes.size());
		//			for (size_t j = 0; j < srcset.size(); j++) {
		//				dstset[j].binding = srcset[j].binding;
		//				dstset[j].descriptorCount = srcset[j].descriptorCount;

		//				dstset[j].descriptorType = srcset[j].descriptorType;
		//				dstset[j].pImmutableSamplers = srcset[j].pImmutableSamplers;
		//				dstset[j].stageFlags |= stage;
		//				ASSERT(dstnames[j].empty() || dstnames[j] == srcnames[j], "Incorrect set binding name!");
		//				dstnames[j] = srcnames[j];
		//				dstsizes[j] = srcsizes[j];
		//			}

		//		}
		//	}
		//	{
		//		auto setbindings = pair.second;
		//		auto& setnames = shaderBindingCombinedNames[stage];
		//		auto& setoffsets = shaderBindingCombinedOffsets[stage];
		//		//for (size_t i = 0; i < shaderBindingCombinedOffsets.size(); i++) {
		//		for (size_t i = 0; i < setbindings.size(); i++) {
		//			auto& srcnames = setnames[i];
		//			auto& srcoffsets = setoffsets[i];
		//			auto& dstnames = mergedCombinedNames[i];
		//			if (dstnames.size() < srcnames.size())
		//				dstnames.resize(srcnames.size());
		//			auto& dstoffsets = mergedCombinedOffsets[i];
		//			if (dstoffsets.size() < srcoffsets.size())
		//				dstoffsets.resize(srcoffsets.size());
		//			for (size_t j = 0; j < srcnames.size();j++) {
		//				dstnames[j] = srcnames[j];
		//				dstoffsets[j] = srcoffsets[j];
		//			}
		//		}
		//	}
		//}
		////merge push constants
		//std::vector<VkPushConstantRange> mergedPushConsts(maxPushConst);
		//std::vector<std::string> mergedPushConstsNames(maxPushConst);
		//for (auto& pair : pushConstRanges) {
		//	VkShaderStageFlagBits stage = pair.first;
		//	auto& srcranges = pair.second;
		//	for(size_t i=0;i<srcranges.size();i++){
		//		auto& srcrange = srcranges[i];
		//		auto& dstrange = mergedPushConsts[i];
		//		dstrange.offset = srcrange.offset;
		//		dstrange.size = srcrange.size;
		//		dstrange.stageFlags |= stage;
		//	}			
		//}
		//for (auto& pair : pushConstNames) {
		//	auto& srcnames = pair.second;
		//	for (size_t i = 0; i < srcnames.size(); i++) {
		//		auto& srcname = srcnames[i];
		//		auto& dst = mergedPushConstsNames[i];
		//		dst = srcname;
		//	}
		//}
		//uint32_t typeIndex = 0;
		////change dynamic types
		//for (auto& set : mergedBindings) {
		//	for (auto& binding : set) {
		//		VkDescriptorType descriptorType = binding.descriptorType;
		//		if (ptypes) {
		//			if (typeIndex > numtypes) {
		//				LOG_WARN("More storage types {0} in shader than expected {1}.", typeIndex, numtypes);
		//			}
		//			switch (ptypes[typeIndex]) {
		//			case Renderer::ShaderStorageType::Uniform:
		//				if (descriptorType != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
		//					LOG_WARN("Expected Uniform at {0}", typeIndex);
		//				break;
		//			case Renderer::ShaderStorageType::UniformDynamic:
		//				if (!(descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC))
		//					LOG_WARN("Expected Dynamic Uniform at {0}", typeIndex);
		//				descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//				break;
		//			case Renderer::ShaderStorageType::Storage:
		//				if (descriptorType != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
		//					LOG_WARN("Expected Storage buffer at {0}", typeIndex);
		//				break;
		//			case Renderer::ShaderStorageType::StorageDynamic:
		//				if (!(descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER || descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))
		//					LOG_WARN("Expected Dynamic Storage buffer at {0}", typeIndex);
		//				descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
		//				break;
		//			case Renderer::ShaderStorageType::Texture:
		//				if (descriptorType != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
		//					LOG_WARN("Expected Combined image sampler at {0}", typeIndex);
		//				break;
		//			default:
		//				LOG_WARN("Unexpected shader type at {0}", typeIndex);
		//				break;
		//			}
		//		}
		//		typeIndex++;
		//		binding.descriptorType = descriptorType;
		//	}
		//	
		//}
		//
		//Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		//Vulkan::VulkContext& context = *contextptr;
		//Vulkan::VulkFrameData* framedataptr = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		//Vulkan::VulkFrameData& framedata = *framedataptr;
		////build descriptor sets
		//std::vector<VkDescriptorSetLayout> layouts;	
		//for (auto& set : mergedBindings) {
		//	auto layoutbuilder = DescriptorSetLayoutBuilder::begin(context.pLayoutCache);
		//	for (auto& binding : set) {
		//		layoutbuilder.AddBinding(binding);
		//	}			
		//	VkDescriptorSetLayout descriptorLayout = layoutbuilder.build();
		//	layouts.push_back(descriptorLayout);
		//	
		//}
		//VkPipelineLayout pipelineLayout;
		//PipelineLayoutBuilder::begin(context.device)
		//	.AddDescriptorSetLayouts(layouts)
		//	.AddPushConstants(mergedPushConsts)
		//	.build(pipelineLayout);

		//std::vector<ShaderModule> shaders;
		//for (auto& pair : spirvMap) {
		//	auto& spirv = pair.second;
		//	VkShaderModule shader = VK_NULL_HANDLE;

		//	VkShaderModuleCreateInfo createInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		//	createInfo.codeSize = spirv.size() * sizeof(uint32_t);
		//	createInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
		//	VkResult res = vkCreateShaderModule(context.device, &createInfo, nullptr, &shader);
		//	ASSERT(res == VK_SUCCESS,"Unable to create shader module!");
		//	shaders.push_back({ shader,pair.first });
		//}
		//VkVertexInputBindingDescription vertexInputDescription;
		//
		//auto& vertexInputs = shaderInputs[VK_SHADER_STAGE_VERTEX_BIT];
		//std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions(vertexInputs.size());
		//uint32_t offset = 0;
		//for(size_t i=0;i<vertexInputs.size();i++){
		//	auto& tuple = vertexInputs[i];
		//	VkFormat format = std::get<1>(tuple);
		//	uint32_t size = std::get<2>(tuple);

		//	vertexAttributeDescriptions[i].location = (uint32_t)i;
		//	vertexAttributeDescriptions[i].offset = offset;
		//	vertexAttributeDescriptions[i].format = format;
		//	offset += size;
		//}
		//vertexInputDescription.binding = 0;
		//vertexInputDescription.stride = offset;
		//vertexInputDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		//
		//{
		//	VkPipeline pipeline = VK_NULL_HANDLE;
		//	PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
		//		.setBlend(enableBlend?VK_TRUE:VK_FALSE)
		//		.setDepthTest(enableDepth?VK_TRUE:VK_FALSE)
		//		.setDepthCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
		//		.setCullMode(cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT)
		//		.setDepthTest(VK_TRUE)//need this to be a parameter
		//		.build(pipeline);
		//	VulkanShaderData shaderData;
		//	shaderData.descriptorSetLayouts = layouts;
		//	
		//	shaderData.pipeline = shaderData.filledPipeline = pipeline;
		//	shaderData.pipelineLayout = pipelineLayout;
		//	_shaderList[name] = shaderData;
		//}
		//{
		//	VkPipeline pipeline = VK_NULL_HANDLE;
		//	//wireframe
		//	PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
		//		.setPolygonMode(VK_POLYGON_MODE_LINE)
		//		.setCullMode(cullBackFaces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT)
		//		.setDepthTest(VK_TRUE)//need this to be a parameter
		//		.build(pipeline);
		//	_shaderList[name].wireframePipeline = pipeline;
		//	
		//	
		//}
		//for (auto& shader : shaders) {
		//	cleanupShaderModule(context.device, shader.shaderModule);
		//}
		////create uniforms, if any
		//std::vector<VkDeviceSize> uniformSizes;
		//std::vector<std::string> uniformNames;
		//std::vector<uint32_t> uboSetBindings;
		//std::vector<VkDeviceSize> uniformDynamicSizes;
		//std::vector<std::string> uniformDynamicNames;
		//std::vector<uint32_t> uniformSetDynamicBindings;
		//std::vector<VkDeviceSize> storageSizes;
		//std::vector<std::string> storageNames;
		//std::vector<uint32_t> storageSetBindings;
		//std::vector<VkDeviceSize> storageDynamicSizes;
		//std::vector<std::string> storageDynamicNames;
		//std::vector<uint32_t> storageSetDynamicBindings;
		//std::vector<std::string> imageNames;
		//std::vector<uint32_t> imageSetBindings;
		//std::vector<uint32_t> imageCounts;
		//std::vector<std::string> combinedNames;
		//std::vector<uint32_t> combinedOffsets;
		//
		//for (uint32_t s = 0; s < mergedBindings.size(); s++) {
		//	auto& set = mergedBindings[s];
		//	for (uint32_t b = 0; b < set.size(); b++) {
		//		auto& binding = set[b];
		//		uint32_t setBinding = s << 16 | b;
		//		if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
		//			uniformSizes.push_back(mergedSizes[s][b]);
		//			uniformNames.push_back(mergedNames[s][b]);					
		//			uboSetBindings.push_back(setBinding);
		//		}
		//		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
		//			uniformDynamicSizes.push_back(mergedSizes[s][b]);//really only the size of 1 item in array
		//			uniformDynamicNames.push_back(mergedNames[s][b]);
		//			uniformSetDynamicBindings.push_back(setBinding);
		//		}
		//		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
		//			storageSizes.push_back(mergedSizes[s][b]);
		//			storageNames.push_back(mergedNames[s][b]);					
		//			storageSetBindings.push_back(setBinding);
		//		}
		//		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
		//			storageDynamicSizes.push_back(mergedSizes[s][b]);
		//			storageDynamicNames.push_back(mergedNames[s][b]);
		//			storageSetDynamicBindings.push_back(setBinding);
		//		}
		//		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
		//			imageNames.push_back(mergedNames[s][b]);
		//			imageSetBindings.push_back(setBinding);
		//			imageCounts.push_back(binding.descriptorCount);
		//		}
		//	}
		//}

		//for (size_t set = 0; set < mergedCombinedNames.size(); set++) {
		//	auto& setNames = mergedCombinedNames[set];
		//	auto& setOffsets = mergedCombinedOffsets[set];
		//	for (size_t n = 0; n < setNames.size(); n++) {
		//		if (std::find(combinedNames.begin(), combinedNames.end(), setNames[n]) == combinedNames.end()) {
		//			combinedNames.push_back(setNames[n]);
		//			combinedOffsets.push_back(setOffsets[n]);
		//		}
		//	}
		//}
		//
		//
		//	//allocate uniforms, can't do with storage buffers as size is determined in app, similar with dynamic buffers
		//	if (uniformSizes.size() > 0) {
		//		std::vector<UniformBufferInfo> uboLayoutInfo;
		//		Vulkan::Buffer uniformBuffer;
		//		auto unibuilder = UniformBufferBuilder::begin(context.device, context.deviceProperties, context.memoryProperties, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, true);
		//		for (auto& size : uniformSizes) {
		//			unibuilder.AddBuffer(size, 1, 1);
		//		}
		//		unibuilder.build(uniformBuffer, uboLayoutInfo);


		//		std::unordered_map<std::string, void*> uboMap;
		//		std::unordered_map<std::string, VkDeviceSize> uboSizeMap;
		//		std::unordered_map<std::string, void*> uniformCombinedMap;
		//		for (size_t i = 0; i < uniformNames.size(); i++) {
		//			void* ptr = uboLayoutInfo[i].ptr;
		//			auto& name = uniformNames[i];
		//			for (size_t n = 0; n < combinedNames.size(); n++) {
		//				auto& comb = combinedNames[n];
		//				auto pos = comb.rfind(name, 0);
		//				if (pos==0) {
		//					uniformCombinedMap[comb.substr(name.length()+1,comb.length()-name.length()-1)] = (void*)((uint8_t*)ptr + combinedOffsets[n]);
		//				}
		//				comb = comb.substr(name.length() + 1, comb.length() - name.length() - 1);
		//			}
		//			uboMap[uniformNames[i]] = uboLayoutInfo[i].ptr;
		//			uboSizeMap[uniformNames[i]] = uniformSizes[i];
		//		}
		//		_shaderList[name].uniformMap = uboMap;
		//		_shaderList[name].uboSizeMap = uboSizeMap;
		//		_shaderList[name].uniformBuffer = uniformBuffer;
		//		_shaderList[name].uniformCombinedMap = uniformCombinedMap;
		//	}
		//	_shaderList[name].uniformNames = uniformNames;
		//	_shaderList[name].uniformSetBindings = uboSetBindings;
		//	_shaderList[name].uniformDynamicNames = uniformDynamicNames;
		//	_shaderList[name].uniformDynamicSetBindings = uniformSetDynamicBindings;

		//	_shaderList[name].imageNames = imageNames;
		//	_shaderList[name].imageSetBindings = imageSetBindings;
		//	_shaderList[name].imageCounts = imageCounts;


		//	_shaderList[name].storageNames = storageNames;
		//	_shaderList[name].storageSetBindings = storageSetBindings;
		//	for (auto& name : storageNames) {//dummy values, do we need a map and not just an array?
		//		_shaderList[name].storageMap[name] = nullptr;
		//		_shaderList[name].storageSizeMap[name] = 0;
		//	}
		//	_shaderList[name].storageDynamicNames = storageDynamicNames;
		//	_shaderList[name].storageSetDynamicBindings = storageSetDynamicBindings;
		//	for (auto& name : storageDynamicNames) {
		//		_shaderList[name].storageDynamicMap[name] = nullptr;
		//		_shaderList[name].storageSizeMap[name] = 0;
		//	}
		//	for (auto& pc : mergedPushConsts) {
		//		_shaderList[name].pushConstStages = pc.stageFlags;
		//	}
		//	_shaderList[name].pushConstNames = mergedPushConstsNames;
		//	_shaderList[name].pushConstRanges = mergedPushConsts;

		//	_shaderList[name].uniformCombinedNames = combinedNames;
		//	
		//

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
	void* VulkanShaderManager::CreateShaderData(const char* shaderPath,bool cullBackFaces,bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes) {
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList.find(name) == _shaderList.end()) {
			std::string source = readFile(filepath);
			const std::unordered_map<VkShaderStageFlagBits, std::string> shaderSources = PreProcess(source);
			CompileShader(name,shaderSources,cullBackFaces,enableBlend,enableDepth,ptypes,numtypes);
		}
		return &_shaderList[name];
	}
}