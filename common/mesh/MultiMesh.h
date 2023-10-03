#pragma once
#include "../Renderer/RenderDevice.h"
#include "../Renderer/Texture.h"
#include "../Renderer/ShaderManager.h"
#include "BaseMesh.h"
#include "MeshTypes.h"
namespace Mesh {
	class MultiMesh : public BaseMesh {
	public:
		static MultiMesh* Create(Renderer::RenderDevice* pdevice,  mat4&xform,std::vector<float>&vertices,std::vector<uint32_t>&indices, uint32_t vertexStride, std::vector<Primitive>& primitives);
		virtual ~MultiMesh() = default;
		virtual uint32_t GetPartCount()const = 0;
		virtual uint32_t GetMaterialIndex(uint32_t part)const = 0;
		//virtual void Bind()const = 0;
		virtual void Render(uint32_t iPart)const=0;
		virtual void GetBoundingBox(vec3& min, vec3& max) = 0;
		virtual void GetBoundingSphere(vec3& center, float& radius) = 0;
		virtual void GetWorldXForm(mat4& world) = 0;
	};
}