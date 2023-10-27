
#include "VulkanLine.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"

namespace Vulkan {
	
	
	VulkanLine::VulkanLine(Renderer::RenderDevice* pdevice, Renderer::LineVertex* pvertices, uint32_t vertexCount, float lineWidth,bool isLineList) :_pdevice(pdevice),_lineWidth(lineWidth)
	{
		
		const char* vertexSrc = R"(
#version 450

layout(location=0) in vec3 inPos;
layout(location=1) in vec4 inColor;

layout(location=0) out vec4 outColor;


layout (push_constant) uniform PushConst{
	mat4 projection;
};

void main(){
	gl_Position = projection * vec4(inPos,1.0);
	outColor = inColor;
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
		LOG_INFO("Compiling Line Shader...");
		//build pipepline first time 
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSpirv;
		std::vector<uint32_t> fragmentSpirv;
		vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
		fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);
		std::vector<uint32_t> vertexLocations;
		VkDeviceSize vertSize = vertexCount * sizeof(Renderer::LineVertex);
		if (vertSize > 0){

			VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
			.AddVertices(vertSize, (float*)pvertices)
			.build(_vertexBuffer, vertexLocations);
		}
		std::vector<VkPushConstantRange> pushConstants{ {VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(PushConst)} };
			
		PipelineLayoutBuilder::begin(context.device)
				
			.AddPushConstants(pushConstants)
			.build(pipelineLayout);
		linePipelineLayoutPtr = std::make_unique<VulkanPipelineLayout>(context.device, pipelineLayout);

		std::vector<ShaderModule> shaders;
		VkVertexInputBindingDescription vertexInputDescription = {};
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;
		ShaderProgramLoader::begin(context.device)
			.AddShaderSpirv(vertexSpirv)			
			.AddShaderSpirv(fragmentSpirv)
			.load(shaders, vertexInputDescription, vertexAttributeDescriptions);
			
			
		PipelineBuilder::begin(context.device, pipelineLayout, framedata.renderPass, shaders, vertexInputDescription, vertexAttributeDescriptions)
			.setTopology(isLineList?VK_PRIMITIVE_TOPOLOGY_LINE_LIST : VK_PRIMITIVE_TOPOLOGY_LINE_STRIP)
			.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
			.setDepthTest(VK_TRUE)
			.build(pipeline);
		linePipelinePtr = std::make_unique<VulkanPipeline>(context.device, pipeline);
		for (auto& shader : shaders) {
			cleanupShaderModule(context.device, shader.shaderModule);
		}
			
		_vertexCount = vertexCount;
		
	}

	VulkanLine::~VulkanLine()
	{
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		cleanupBuffer(context.device, _vertexBuffer);
		linePipelineLayoutPtr.reset();
		linePipelinePtr.reset();
		
	}
	
	void VulkanLine::Draw(glm::mat4&viewProj)
	{
		if (_vertexCount) {
			pushConst.projection = viewProj;

			VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
			VulkFrameData& framedata = *framedataptr;
			VkCommandBuffer cmd = framedata.cmd;
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(cmd, 0, 1, &_vertexBuffer.buffer, offsets);
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConst), &pushConst);
			vkCmdSetLineWidth(cmd, _lineWidth);
			vkCmdDraw(cmd, _vertexCount, 1, 0, 0);
		}
	}
	void VulkanLine::ResetVertices(Renderer::LineVertex* pvertices, uint32_t vertexCount)
	{
		//could resuse buffer if new data smaller or same size
		VulkContext* contextptr = reinterpret_cast<VulkContext*>(_pdevice->GetDeviceContext());
		VulkContext& context = *contextptr;
		VkDeviceSize vertSize = (VkDeviceSize)(vertexCount * sizeof(Renderer::LineVertex));
		if (vertexCount > 0) {
			if (vertexCount > _vertexCount || vertSize > _vertexBuffer.size) {
				cleanupBuffer(context.device, _vertexBuffer);
				std::vector<uint32_t> vertexLocations;
				VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
					.AddVertices(vertSize, (float*)pvertices)
					.build(_vertexBuffer, vertexLocations);

			}
			else {
				Vulkan::Buffer stagingBuffer = StagingBufferBuilder::begin(context.device, context.memoryProperties)
					.setSize(vertSize)
					.build();
				void* ptr = mapBuffer(context.device, stagingBuffer);
				memcpy(ptr, pvertices, vertSize);
				unmapBuffer(context.device, stagingBuffer);
				CopyBufferTo(context.device, context.queue, context.commandBuffer, stagingBuffer, _vertexBuffer, vertSize);

			}
		}
		else {

		}
		_vertexCount = vertexCount;
	}
}