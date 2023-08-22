#pragma once
#include "VulkanParticleSwarm.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"
Renderer::ParticleSwarm* Renderer::ParticleSwarm::Create(Renderer::RenderDevice*pdevice,Renderer::ParticleVertex*pvertices,int vertexCount,glm::vec2&size) {
	return new Vulkan::VulkanParticleSwarm(pdevice,pvertices,vertexCount,size);
}
namespace Vulkan {
	
	
	VulkanParticleSwarm::VulkanParticleSwarm(Renderer::RenderDevice* pdevice, Renderer::ParticleVertex* pvertices, int vertexCount,glm::vec2&size):_pdevice(pdevice)
	{
		pushConst.size = size;
		const char* vertexSrc = R"(
#version 450

layout(location=0) in vec3 inPos;
layout(location=1) in vec4 inColor;


layout(location=0) out vec3 PosW;
layout(location=1) out vec4 outColor;


void main(){
	// Just pass data over to geometry shader.
	PosW = inPos;		
	outColor=inColor;
}

)";

		const char* geometrySrc = R"(
#version 450 
layout (points) in;
layout (triangle_strip,max_vertices=4) out;


layout(location=0) in vec3 inCenterW[];
layout(location=1) in vec4 inColor[];

layout(location=0) out vec4 outColor;

layout (push_constant) uniform PushConst{	
	mat4 viewProj;
	vec3 eyePosW;
	vec2 sizeW;	
};


void main(){
//
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	//
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	vec3 look = eyePosW - inCenterW[0];
	look.y = 0.0f; // y-axis aligned, so project to xz-plane
	look = normalize(look);
	vec3 right = cross(up, look);
	
	//
	// Compute triangle strip vertices (quad) in world space.
	//
	float halfWidth  = 0.5f*sizeW.x;
	float halfHeight = 0.5f*sizeW.y;
	
	vec4 v[4];
	v[0] = vec4(inCenterW[0] + halfWidth*right - halfHeight*up, 1.0f);
	v[1] = vec4(inCenterW[0] + halfWidth*right + halfHeight*up, 1.0f);
	v[2] = vec4(inCenterW[0] - halfWidth*right - halfHeight*up, 1.0f);
	v[3] = vec4(inCenterW[0] - halfWidth*right + halfHeight*up, 1.0f);

	//
	// Transform quad vertices to world space and output 
	// them as a triangle strip.
	//
	for(int i=0;i<4;++i){
		gl_Layer=0;//one face?		
		gl_Position = viewProj*v[i];				
		outColor = inColor[0];
		EmitVertex();
	}
	EndPrimitive();    
}
)";
		const char* fragmentSrc = R"(
#version 450
layout(location=0) in vec4 inColor;
layout(location=0) out vec4 FragColor;

void main(){
	FragColor = inColor;
}
)";
		_pdevice = pdevice;
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		
		//build pipepline first time 
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSpirv;
		std::vector<uint32_t> fragmentSpirv;
		std::vector<uint32_t> geometrySpirv;
		vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
		geometrySpirv = compiler.compileShader(geometrySrc, VK_SHADER_STAGE_GEOMETRY_BIT);
		fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);
		std::vector<uint32_t> vertexLocations;
		VkDeviceSize vertSize = vertexCount * sizeof(Renderer::ParticleVertex);
		VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
			.AddVertices(vertSize, (float*)pvertices)
			.build(_vertexBuffer, vertexLocations);

		std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_GEOMETRY_BIT,0,sizeof(PushConst)} };
			
		PipelineLayoutBuilder::begin(context.device)
				
			.AddPushConstants(pushConstants)
			.build(pipelineLayout);
		particlePipelineLayoutPtr = std::make_unique<VulkanPipelineLayout>(context.device, pipelineLayout);

		std::vector<ShaderModule> shaders;
		VkVertexInputBindingDescription vertexInputDescription = {};
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		ShaderProgramLoader::begin(context.device)
			.AddShaderSpirv(vertexSpirv)
			.AddShaderSpirv(geometrySpirv)
			.AddShaderSpirv(fragmentSpirv)
			.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
			
			
		PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
			.setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
			.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.setDepthTest(VK_TRUE)
			.build(pipeline);
		particlePipelinePtr = std::make_unique<VulkanPipeline>(context.device, pipeline);
		for (auto& shader : shaders) {
			cleanupShaderModule(context.device, shader.shaderModule);
		}
			
		_vertexCount = vertexCount;
		
	}

	VulkanParticleSwarm::~VulkanParticleSwarm()
	{
		
		particlePipelineLayoutPtr.release();
		particlePipelinePtr.release();
		
	}
	
	void VulkanParticleSwarm::Draw(glm::mat4&viewProj,glm::vec3&eyePos)
	{
		pushConst.viewProj = viewProj;
		pushConst.eyePos = eyePos;

		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd, 0, 1, &_vertexBuffer.buffer, offsets);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_GEOMETRY_BIT, 0, sizeof(PushConst), &pushConst);
		vkCmdDraw(cmd, _vertexCount, 1, 0, 0);
	}
}