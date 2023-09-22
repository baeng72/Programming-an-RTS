#pragma once
#include "../Renderer/RenderDevice.h"
#include "../Renderer/Texture.h"
#include "../Renderer/ShaderManager.h"
#include "MeshTypes.h"
namespace Mesh {
	class MultiMesh {
	public:
		static MultiMesh* Create(Renderer::RenderDevice* pdevice, std::shared_ptr<Renderer::ShaderManager>& shaderManager, mat4&xform,std::vector<float>&vertices,std::vector<uint32_t>&indices, uint32_t vertexStride, std::vector<Primitive>& primitives, std::vector<std::unique_ptr<Renderer::Texture>>& textures);
		virtual ~MultiMesh() = default;
		virtual void Render(void* pubo, uint32_t uboSize, void* ppushc, uint32_t pushcsize)=0;
		virtual void GetBoundingBox(vec3& min, vec3& max) = 0;
		virtual void GetBoundingSphere(vec3& center, float& radius) = 0;
		virtual void GetWorldXForm(mat4& world) = 0;
	};
}