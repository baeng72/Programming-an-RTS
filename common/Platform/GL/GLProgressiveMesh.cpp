#include "GLProgressiveMesh.h"
#include <meshoptimizer/src/meshoptimizer.h>
#include "../../Core/log.h"

namespace GL {
	GLProgressiveMesh::GLProgressiveMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes)
		:_pdevice(pdevice)
	{
		uint32_t vertStride = vertexAttributes.vertexStride;
		uint32_t indCount = indSize / sizeof(uint32_t);
		uint32_t vertCount = vertSize / vertStride;
		std::vector<uint32_t> indices(pindices, pindices + indCount);
		std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
		size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, vertStride);
		_rawIndices.resize(indCount);
		meshopt_remapIndexBuffer(_rawIndices.data(), pindices, indCount, remap.data());
		_rawVertices.resize(vertCount * vertStride);
		meshopt_remapVertexBuffer(_rawVertices.data(), pvertices, vertCount, vertStride, remap.data());
		glGenVertexArrays(1, &_vao);
		glGenBuffers(1, &_vertexBuffer);
		glGenBuffers(1, &_indexBuffer);
		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertSize, _rawVertices.data(), GL_STATIC_DRAW);
		size_t offset = 0;
		for (size_t i = 0; i < vertexAttributes.vertexAttribs.size(); i++) {
			size_t size = 0;
			size_t count = 0;
			GLenum type = GL_FLOAT;
			switch (vertexAttributes.vertexAttribs[i]) {
			case Renderer::ShaderDataType::Float:
				size = sizeof(float);
				count = 1;
				type = GL_FLOAT;
				break;
			case Renderer::ShaderDataType::Float2:
				size = sizeof(float);
				count = 2;
				type = GL_FLOAT;
				break;
			case Renderer::ShaderDataType::Float3:
				size = sizeof(float);
				count = 3;
				type = GL_FLOAT;
				break;
			case Renderer::ShaderDataType::Float4:
				size = sizeof(float);
				count = 4;
				type = GL_FLOAT;
				break;
			default:
				assert(0);
				break;

			}
			glVertexAttribPointer((GLint)i,(GLint) size,(GLenum) type, GL_FALSE, vertStride, (void*)(offset));
			glEnableVertexAttribArray((GLuint)i);
			offset += size * count;
		}
		_rawVertexCount = vertSize / vertStride;
		_vertexStride = vertStride;
		_vertSize = vertSize;

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, _rawIndices.data(), GL_STATIC_DRAW);
		_indSize = indSize;
		_rawIndexCount = _currIndexCount = indSize / sizeof(uint32_t);
		

	}
	GLProgressiveMesh::~GLProgressiveMesh()
	{
		glDeleteBuffers(1, &_indexBuffer);
		glDeleteBuffers(1, &_vertexBuffer);
		glDeleteVertexArrays(1, &_vao);
	}
	void GLProgressiveMesh::SetIndexCount(uint32_t indexCount)
	{
		if (indexCount == _rawIndexCount)
			return;
		if (indexCount > _rawIndexCount) {
			indexCount = _rawIndexCount;
		}

		float resultError = 0;
		float targetError = 1e-2f;
		std::vector<uint32_t> lodIndices(_rawIndexCount);
		lodIndices.resize(meshopt_simplify(lodIndices.data(), _rawIndices.data(), _rawIndexCount, _rawVertices.data(), _rawVertexCount, _vertexStride, indexCount, 0.1f, 0, &resultError));
		uint32_t lodIndexCount = (uint32_t)lodIndices.size();
		uint32_t lodIndexSize = lodIndexCount * sizeof(uint32_t);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, lodIndexSize, lodIndices.data());
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, lodIndexSize, lodIndices.data(), GL_STATIC_DRAW);
		_currIndexCount = lodIndexCount;
	}
	uint32_t GLProgressiveMesh::GetIndexCount()
	{
		return _currIndexCount;
	}
	void GLProgressiveMesh::Render()
	{
		glBindVertexArray(_vao);
		glDrawElements(GL_TRIANGLES, _currIndexCount, GL_UNSIGNED_INT, nullptr);
	}
}