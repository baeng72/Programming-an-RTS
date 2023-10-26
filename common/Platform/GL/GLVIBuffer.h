#pragma once
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/VIBuffer.h"
#include "../../Renderer/ShaderTypes.h"



namespace GL {
	class GLVIBuffer : public Renderer::VIBuffer {
		Renderer::RenderDevice* _pdevice;
		unsigned int _vao;
		unsigned int _vertexBuffer;
		unsigned int _indexBuffer;
		uint32_t _indexCount;
		size_t	_hash;
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes);
	public:
		GLVIBuffer(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& attributes, bool optimize = false);
		~GLVIBuffer();
		virtual void Bind()override;
		virtual size_t GetHash()override;
		virtual void Render()override;
	};
}