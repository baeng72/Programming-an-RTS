#pragma once
#include "../../mesh/MultiMesh.h"
#include "../../Renderer/Shader.h"
#include "../../Renderer/Texture.h"
#include "VulkanEx.h"
namespace Vulkan {
	class VulkanMultiMesh : public Mesh::MultiMesh {
		Renderer::RenderDevice* _pdevice;
		Vulkan::Buffer _vertexBuffer;
		Vulkan::Buffer _indexBuffer;
		std::vector<std::unique_ptr<Renderer::Texture>> _textures;
		std::vector<std::unique_ptr<Renderer::Shader>> _shaders;
		std::vector<Mesh::Primitive> _primitives;
		std::vector<Mesh::Material> _materials;
		mat4 _world;
		vec3 _min;
		vec3 _max;
		vec3 _center;
		float _radius;
		void CreateShaders(std::shared_ptr<Renderer::ShaderManager>& shaderManager);
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
	public:
		VulkanMultiMesh(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t vertexStride, std::vector<Mesh::Primitive>& primitives, std::vector<std::unique_ptr<Renderer::Texture>>& textures);
		virtual ~VulkanMultiMesh();
		virtual void Render(void* pubo, uint32_t ubosize, void* ppushc, uint32_t pushcsize)override;
		virtual void GetBoundingBox(vec3& min, vec3& max)override;
		virtual void GetBoundingSphere(vec3& center, float& radius)override;
		virtual void GetWorldXForm(mat4& world) override;
	};
}