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
		//std::vector<std::unique_ptr<Renderer::Texture>> _textures;
		//std::vector<std::unique_ptr<Renderer::Shader>> _shaders;
		std::vector<Mesh::Primitive> _primitives;
		//std::vector<Mesh::Material> _materials;
		mat4 _world;
		vec3 _min;
		vec3 _max;
		vec3 _center;
		float _radius;
		size_t _hash;
		/*void CreateShaders(std::shared_ptr<Renderer::ShaderManager>& shaderManager);*/
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize);
	public:
		VulkanMultiMesh(Renderer::RenderDevice* pdevice,  mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, uint32_t vertexStride, std::vector<Mesh::Primitive>& primitives);
		virtual ~VulkanMultiMesh();
		virtual uint32_t GetPartCount()const override;
		virtual uint32_t GetMaterialIndex(uint32_t part)const override;
		virtual void Bind() override;
		virtual size_t GetHash()override;
		virtual void Render(uint32_t iPart)const  override;
		virtual void GetBoundingBox(vec3& min, vec3& max)override;
		virtual void GetBoundingSphere(vec3& center, float& radius)override;
		virtual void GetWorldXForm(mat4& world) override;
	};
}