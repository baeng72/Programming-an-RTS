#pragma once
#include "../../mesh/Mesh.h"
#include <glad/glad.h>

namespace GL {
	class GLMesh : public Mesh::Mesh {
		Renderer::RenderDevice* _pdevice;
		GLuint _vertexBuffer;
		GLuint _indexBuffer;
		GLuint _vao;
		uint32_t		_indexCount;
		size_t	_hash;
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize,Renderer::VertexAttributes&vertexAttributes);
	public:
		GLMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize,Renderer::VertexAttributes&vertexAttributes, bool optimize);
		//GLMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertStride, uint32_t vertCount, uint32_t* pindices, uint32_t indSize, bool optimize = true);
		virtual ~GLMesh();
		void Render()override;
		void Bind()override;
		size_t GetHash()override;
	};
}