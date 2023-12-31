#include "GLMesh.h"
#include "../../Core/Log.h"
#include "../../Core/hash.h"
#include "../meshoptimizer/src/meshoptimizer.h"
#include "GLERR.h"
namespace GL {
	
	/*GLMesh::GLMesh(Renderer::RenderDevice* pdevice, float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize)
	{
		Create(pvertices, vertSize, pindices, indSize);
	}*/
	GLMesh::GLMesh(Renderer::RenderDevice* pdevice, float* pvertices,  uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes&vertexAttributes,bool optimize)
		:_pdevice(pdevice)
	{
		uint32_t vertStride = vertexAttributes.vertexStride;
		uint32_t vertCount = vertSize / vertStride;
		if (optimize) {
			

			uint32_t indCount = indSize / sizeof(uint32_t);

			std::vector<uint32_t> indices(pindices, pindices + indCount);

			std::vector<unsigned int> remap(indCount); // allocate temporary memory for the remap table
			size_t vertex_count = meshopt_generateVertexRemap(remap.data(), pindices, indCount, pvertices, vertCount, vertStride);
			std::vector<uint32_t> progressiveIndices(indCount);
			meshopt_remapIndexBuffer(progressiveIndices.data(), pindices, indCount, remap.data());
			std::vector<float> progressiveVertices(vertCount * vertStride);
			meshopt_remapVertexBuffer(progressiveVertices.data(), pvertices, vertCount, vertStride, remap.data());
			Create(progressiveVertices.data(), vertSize, progressiveIndices.data(), indSize, vertexAttributes);
		}
		else {
			Create(pvertices, vertSize, pindices, indSize, vertexAttributes);
		}
		
	}
	GLMesh::~GLMesh()
	{
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vertexBuffer);
		glDeleteBuffers(1, &_indexBuffer);
	}
	void GLMesh::Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize,Renderer::VertexAttributes&vertexAttributes)
	{
		_vao = _vertexBuffer = _indexBuffer = 0;
		_hash = 0;
		_hash = Core::HashFNV1A(pvertices, vertSize);
		
		glGenVertexArrays(1, &_vao);
		GLERR();
		
		
		glBindVertexArray(_vao);
		GLERR();
		glGenBuffers(1, &_vertexBuffer);
		GLERR();
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		GLERR();
		glBufferData(GL_ARRAY_BUFFER, vertSize, pvertices, GL_STATIC_DRAW);
		GLERR();
		/*struct Vertex {
			vec3 pos;
			vec3 norm;
			vec4 color;
		};*/
		 
		GLuint ioffset = 0;
		for (size_t i = 0; i < vertexAttributes.vertexAttribs.size(); i++) {
			GLuint count = 0;
			GLuint size = sizeof(float);
			GLenum type = GL_FLOAT;
			switch (vertexAttributes.vertexAttribs[i]) {
			case Renderer::ShaderDataType::Float:
				count = 1;
				type = GL_FLOAT;
				size = sizeof(float);
				break;
			case Renderer::ShaderDataType::Float2:
				count = 2;
				type = GL_FLOAT;
				size = sizeof(float);
				break;
			case Renderer::ShaderDataType::Float3:
				count = 3;
				type = GL_FLOAT;
				size = sizeof(float);
				break;
			case Renderer::ShaderDataType::Float4:
				count = 4;
				type = GL_FLOAT;
				size = sizeof(float);
				break;
			case Renderer::ShaderDataType::Int4:
				count = 4;
				type = GL_INT;
				size = sizeof(uint32_t);
				break;
			default:
				assert(0);
				break;
			}
			glVertexAttribPointer((GLuint)i, count, type, GL_FALSE, vertexAttributes.vertexStride,(void*) ioffset);			
			ioffset += count * size;
		}
		for (size_t i = 0; i < vertexAttributes.vertexAttribs.size(); i++) {
			glEnableVertexAttribArray((GLuint)i);
		}
		GLERR();
		glGenBuffers(1, &_indexBuffer);
		GLERR();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		GLERR();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, pindices, GL_STATIC_DRAW);
		
		GLERR();
		_indexCount = indSize / sizeof(uint32_t);
		/*uint32_t* pind = new uint32_t[indSize];
		glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indSize, pind);
		uint32_t* poind = (uint32_t*)pindices;
		for (int i = 0; i < _indexCount; i++) {
			uint32_t i0 = poind[i];
			uint32_t i1 = pind[i];
			if (i0 != i1) {
				int z = 0;
			}
		}
		delete[] pind;*/
		
	}
	void GLMesh::Render()
	{		
		glDrawElements(GL_TRIANGLES, _indexCount, GL_UNSIGNED_INT, 0);
		GLERR();
	}
	void GLMesh::Bind()
	{
		glBindVertexArray(_vao);
		GLERR();
	}
	size_t GLMesh::GetHash()
	{
		return _hash;
	}
	void* GLMesh::GetNativeHandle()
	{
		return (void*)&_vao;
	}
}