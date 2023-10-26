
#include "VulkanLine2D.h"
#include "VulkState.h"
#include "VulkSwapchain.h"
#include "ShaderCompiler.h"
#include "../../Core/Log.h"

namespace Vulkan {
	
	
	VulkanLine2D::VulkanLine2D(Renderer::RenderDevice* pdevice) :_pdevice(pdevice)
	{
		
		
		const char* vertexSrc = R"(
#version 450

layout (location=0) out vec4 outColor;

layout(push_constant) uniform Proj {
	mat4 projection;
	vec2 start;
	vec2 end;
	vec4 color;
};

void main(){	
	outColor=color;
	vec2 verts[2]={start,end};
	gl_Position = projection * vec4(verts[gl_VertexIndex],0.0,1.0);
	
}

)";


		const char* fragmentSrc = R"(
#version 450

layout (location=0) in vec4 inColor;

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
		LOG_INFO("Compiling Line2D Shader...");
		//build pipepline first time 
		ShaderCompiler compiler;
		std::vector<uint32_t> vertexSpirv;
		std::vector<uint32_t> fragmentSpirv;
		vertexSpirv = compiler.compileShader(vertexSrc, VK_SHADER_STAGE_VERTEX_BIT);
		fragmentSpirv = compiler.compileShader(fragmentSrc, VK_SHADER_STAGE_FRAGMENT_BIT);
		
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
			.setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP)
			.setFrontFace(VK_FRONT_FACE_CLOCKWISE)
			
			.setDepthTest(VK_TRUE)
			.build(pipeline);
		linePipelinePtr = std::make_unique<VulkanPipeline>(context.device, pipeline);
		for (auto& shader : shaders) {
			cleanupShaderModule(context.device, shader.shaderModule);
		}
			
		
		
	}

	VulkanLine2D::~VulkanLine2D()
	{
		
		linePipelineLayoutPtr.reset();
		linePipelinePtr.reset();
		
	}

	

	void VulkanLine2D::Update(int width, int height)
	{
		pushConst.projection = glm::ortho(0.f, static_cast<float>(width), 0.f, static_cast<float>(height), -1.f, 1.f);
	}
	
	void VulkanLine2D::Draw(vec2* pvertices, uint32_t count, vec4 color, float width)
	{
		if (count < 2)
			return;
		VulkFrameData* framedataptr = reinterpret_cast<VulkFrameData*>(_pdevice->GetCurrentFrameData());
		VulkFrameData& framedata = *framedataptr;
		VkCommandBuffer cmd = framedata.cmd;
		vkCmdSetLineWidth(cmd, width);
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		pushConst.color = color;
		
		for (uint32_t i = 1; i < count; i++) {
			pushConst.start = pvertices[i - 1];
			pushConst.end = pvertices[i];
			vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConst), &pushConst);
			vkCmdDraw(cmd, 2, 1, 0, 0);
		}
		
	}
	
}