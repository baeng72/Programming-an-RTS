#include "VulkanMultiMesh.h"
#include "VulkSwapchain.h"
#include "VulkState.h"
#include "../../Core/Log.h"
namespace Mesh {
	MultiMesh* MultiMesh::Create(Renderer::RenderDevice* pdevice,  mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t vertexStride, std::vector<Mesh::Primitive>& primitives) {
		return new Vulkan::VulkanMultiMesh(pdevice,xform, vertices,indices,vertexStride, primitives);
	}
}

namespace Vulkan {
	VulkanMultiMesh::VulkanMultiMesh(Renderer::RenderDevice* pdevice, mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t vertexStride, std::vector<Mesh::Primitive>& primitives)
		:_pdevice(pdevice),_primitives(primitives){
		LOG_INFO("Creating MultiMesh vertex/index data");
		Create(vertices.data(), (uint32_t)vertices.size() * sizeof(float), indices.data(), (uint32_t)indices.size()*sizeof(uint32_t));
		
		_world = xform;
		uint32_t vertexCount = (uint32_t)vertices.size() / vertexStride;
		
		{
			float* ppos = vertices.data();
			//calculate bounding box
			_min = vec3(10000.f);
			_max = vec3(-10000.f);
			for (size_t i = 0; i < vertexCount; i++) {

				vec3 pos = *((vec3*)ppos);
				ppos += vertexStride;
				vec3 xpos = vec3(xform * vec4(pos, 1.f));
				//Check if vertex is outside the bounds 
				//if so, then update bounding volume
				_min.x = std::min(_min.x, xpos.x);
				_min.y = std::min(_min.y, xpos.y);
				_min.z = std::min(_min.z, xpos.z);
				_max.x = std::max(_max.x, xpos.x);
				_max.y = std::max(_max.y, xpos.y);
				_max.z = std::max(_max.z, xpos.z);
			}
			_min = _min;
			_max = _max;
		}
		
		{
			float* ppos = vertices.data();
			//calculate bounding sphere
			_center = (_max + _min) / 2.f;//midpoint
			_radius = 0.f;
			for (size_t i = 0; i < vertexCount; i++) {
				vec3 pos = *((vec3*)ppos);
				ppos += vertexStride;
				vec3 xpos = vec3(xform * vec4(pos, 1.f));
				float l = glm::length((xpos - _center));
				_radius = std::max(_radius, l);
			}
		}
	}


	VulkanMultiMesh::~VulkanMultiMesh()
	{
	}

	uint32_t VulkanMultiMesh::GetPartCount()const {
		return (uint32_t)_primitives.size();
	}
	uint32_t VulkanMultiMesh::GetMaterialIndex(uint32_t part)const {
		return _primitives[part].materialIndex;
	}
	void VulkanMultiMesh::Bind()const {
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;
		vkCmdBindIndexBuffer(frameData.cmd, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(frameData.cmd, 0, 1, &_vertexBuffer.buffer, offsets);
	}

	void VulkanMultiMesh::Render(uint32_t part)const
	{
		Vulkan::VulkFrameData* pframedata = reinterpret_cast<Vulkan::VulkFrameData*>(_pdevice->GetCurrentFrameData());
		Vulkan::VulkFrameData& frameData = *pframedata;		
		
		auto& primitive = _primitives[part];
		vkCmdDrawIndexed(frameData.cmd, primitive.indexCount, 1, primitive.indexStart, primitive.vertexStart, 0);
		
	}

	void VulkanMultiMesh::GetBoundingBox(vec3& min, vec3& max)
	{
		min = _min;
		max = _max;
	}

	void VulkanMultiMesh::GetBoundingSphere(vec3& center, float& radius)
	{
		center = _center;
		radius = _radius;
	}

	void VulkanMultiMesh::GetWorldXForm(mat4& world)
	{
		world = _world;
	}
	
	void VulkanMultiMesh::Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize) {
		Vulkan::VulkContext* contextptr = reinterpret_cast<Vulkan::VulkContext*>(_pdevice->GetDeviceContext());
		Vulkan::VulkContext& context = *contextptr;
		{
			std::vector<uint32_t> vertexLocations;
			Vulkan::VertexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddVertices(vertSize, pvertices)
				.build(_vertexBuffer, vertexLocations);
		}
		{
			std::vector<uint32_t> indexLocations;
			Vulkan::IndexBufferBuilder::begin(context.device, context.queue, context.commandBuffer, context.memoryProperties)
				.AddIndices(indSize, pindices)
				.build(_indexBuffer, indexLocations);
		}
	}

	/*void VulkanMultiMesh::CreateShaders(std::shared_ptr<Renderer::ShaderManager>& shaderManager) {
		_shaders.resize(_textures.size());
		
		void* pshaderData = shaderManager->CreateShaderData("../../../../Resources/shaders/mesh_texture.glsl");
		for (size_t i = 0; i < _textures.size();i++) {
			_shaders[i] = std::move(std::unique_ptr<Renderer::Shader>(Renderer::Shader::Create(_pdevice,pshaderData)));
			int texid = 0;
			Renderer::Texture* textures[1] = { _textures[i].get() };
			_shaders[i]->SetTexture(texid, textures, 1);
		}
	}*/

}