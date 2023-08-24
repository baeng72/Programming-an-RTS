#pragma once
#include "VulkanSprite.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"
Renderer::Sprite* Renderer::Sprite::Create(Renderer::RenderDevice*pdevice) {
	return new Vulkan::VulkanSprite(pdevice);
}
namespace Vulkan {
	
	std::unique_ptr<VulkanPipelineLayout> VulkanSprite::spritePipelineLayoutPtr;
	std::unique_ptr<VulkanPipeline> VulkanSprite::spritePipelinePtr;
	int VulkanSprite::instances = 0;
	
	VulkanSprite::VulkanSprite(Renderer::RenderDevice* pdevice):_pdevice(pdevice),_ptexture(nullptr)
	{
		const char* vertexSrc = R"(
#version 450
layout (location=0) out vec2 outUV;

vec3 vertices[4]={
	{0.0,0.0,0.0},
	{ 1.0,0.0,0.0},
	{ 1.0, 1.0,0.0},
	{ 0.0, 1.0,0.0},
};

vec2 uvs[4]={
	{0.0,0.0},
	{1.0,0.0},
	{1.0,1.0},
	{0.0,1.0},
};

int indices[6] = {
	0,1,2,0,2,3
};

layout (push_constant) uniform PushConst{
	mat4 projection;	
	mat4 model;
};

void main(){
	vec3 inPos = vertices[indices[gl_VertexIndex]];
	vec2 inUV = uvs[indices[gl_VertexIndex]];
	gl_Position = projection * model * vec4(inPos,1.0);
	outUV = inUV;
}

)";
		const char* fragmentSrc = R"(
#version 450
layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout (set=0,binding=0) uniform sampler2D text;

void main(){
	vec4 sampled = texture(text,inUV);
	outFragColor = sampled;
}
)";
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		std::vector<VkDescriptorSet> descriptorSetList;
		DescriptorSetBuilder::begin(context.pPoolCache, context.pLayoutCache)
			.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT)
			.build(descriptorSetList, _descriptorLayout,MAX_FRAMES);
		spriteDescriptorPtr = std::make_unique<VulkanDescriptorList>(context.device, descriptorSetList);
		_descriptorSets[0] = descriptorSetList[0];
		_descriptorSets[1] = descriptorSetList[1];
		_descriptorIndex = UINT32_MAX;
		if (instances == 0) {
			//build pipepline first time 
			ShaderCompiler compiler;
			std::vector<uint32_t> vertexSpirv;
			std::vector<uint32_t> fragmentSpirv;
			vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
			fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);

			std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConst)} };
			
			PipelineLayoutBuilder::begin(context.device)
				.AddDescriptorSetLayout(_descriptorLayout)
				.AddPushConstants(pushConstants)
				.build(_pipelineLayout);
			spritePipelineLayoutPtr = std::make_unique<VulkanPipelineLayout>(context.device, _pipelineLayout);

			std::vector<ShaderModule> shaders;
			VkVertexInputBindingDescription vertexInputDescription = {};
			std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
			ShaderProgramLoader::begin(context.device)
				.AddShaderSpirv(vertexSpirv)
				.AddShaderSpirv(fragmentSpirv)
				.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
			
			
			PipelineBuilder::begin(context.device, _pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
				.setBlend(VK_TRUE)
				//.setCullMode(VK_CULL_MODE_FRONT_BIT)
				.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
				.build(_pipeline);
			spritePipelinePtr = std::make_unique<VulkanPipeline>(context.device, _pipeline);
			for (auto& shader : shaders) {
				cleanupShaderModule(context.device, shader.shaderModule);
			}
			
		}
		else {
			_pipelineLayout = *spritePipelineLayoutPtr;
			_pipeline = *spritePipelinePtr;
		}
		instances++;
		pdevice->GetDimensions(&_width, &_height);
		_orthoproj = glm::ortho(0.f, (float)_width, 0.f, (float)_height, -1.f, 1.f);
	}

	VulkanSprite::~VulkanSprite()
	{
		if (--instances == 0) {
			spritePipelineLayoutPtr.release();
			spritePipelinePtr.release();
		}
	}
	
	void VulkanSprite::Draw(Renderer::Texture* ptexture, glm::vec3 position)
	{
		if (ptexture) {
			Vulkan::Texture* ptext = (Vulkan::Texture*)ptexture->GetNativeHandle();
			if (_ptexture != ptext) {
				if (_ptexture && !ptext) {
					//resuse existing
				}
				else {
					_descriptorIndex++;
					_descriptorIndex %= 2;
					ASSERT(ptexture, "Invalid Sprite Texture");
					_scale = ptexture->GetScale();
					VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
					VulkContext& context = *contextptr;
					_ptexture = ptext;
					VkDescriptorImageInfo imageInfo{};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = ptext->imageView;
					imageInfo.sampler = ptext->sampler;
					DescriptorSetUpdater::begin(context.pLayoutCache, _descriptorLayout, _descriptorSets[_descriptorIndex])
						.AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo)
						.update();
				}
			}

			VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
			VulkFrameData& framedata = *framedataptr;
			VkCommandBuffer cmd = framedata.cmd;

			glm::mat4 id = glm::mat4(1.f);
			glm::mat4 t = glm::translate(id, position);
			glm::mat4 s = glm::scale(id, glm::vec3(_ptexture->width * _scale.x, _ptexture->height * _scale.y, 0.f));
			glm::mat4 model = t * s;
			PushConst pushConst = { _orthoproj ,model };

			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSets[_descriptorIndex], 0, nullptr);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
			vkCmdPushConstants(cmd, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConst), &pushConst);
			vkCmdDraw(cmd, 6, 1, 0, 0);
		}
	}
}