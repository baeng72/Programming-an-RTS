#include "../../mesh/ProgressiveMesh.h"
#include <glad/glad.h>

namespace GL {

	class GLProgressiveMesh : public Mesh::ProgressiveMesh {
		Renderer::RenderDevice* _pdevice;
		GLuint _vao;
		GLuint _vertexBuffer;
		GLuint _indexBuffer;
		uint32_t _vertSize;
		uint32_t _indSize;

		uint32_t _vertexStride;
		uint32_t _rawVertexCount;
		uint32_t _rawIndexCount;
		uint32_t _currIndexCount;
		//uint32_t _currOffset;
		//uint32_t* _pIndexBuffer;
		//keep track of original data
		std::vector<float> _rawVertices;
		std::vector<uint32_t> _rawIndices;

	public:
		GLProgressiveMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize,  uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes);
		virtual ~GLProgressiveMesh();
		virtual void SetIndexCount(uint32_t indexCount)override;
		virtual uint32_t GetIndexCount()override;
		virtual void Render()override;
	};

}