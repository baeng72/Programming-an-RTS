#include "GLMultiMesh.h"
#include "../../Core/Log.h"
#include "../../Core/hash.h"
#include "GLERR.h"
namespace GL {
	
	GLMultiMesh::GLMultiMesh(Renderer::RenderDevice* pdevice, mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, std::vector<Mesh::Primitive>& primitives, Renderer::VertexAttributes& vertexAttributes)
		:_pdevice(pdevice), _primitives(primitives),_vao(0),_vertexBuffer(0),_indexBuffer(0) {
		LOG_INFO("Creating GL MultiMesh vertex/index data");
		uint32_t vertexStride = vertexAttributes.vertexStride;
		Create(vertices.data(), (uint32_t)vertices.size() * sizeof(float), indices.data(), (uint32_t)indices.size() * sizeof(uint32_t),vertexAttributes);

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
		{
			uint32_t indSize = (uint32_t)(sizeof(uint32_t) * indices.size());
			uint32_t vertSize = (uint32_t)vertices.size();
			uint32_t primSize = (uint32_t)primitives.size();
			size_t vhash = HASH(&vertSize);
			size_t ihash = HASH(&indSize);
			size_t shash = HASH(&vertexStride);
			size_t phash = HASH(&primSize);
			_hash = (vhash * Core::fnvprime) ^ (ihash * Core::fnvprime) ^ (shash * Core::fnvprime) ^ phash;
		}
	
	}
	GLMultiMesh::~GLMultiMesh()
	{
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vertexBuffer);
		glDeleteBuffers(1, &_indexBuffer);
	}
	void GLMultiMesh::Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes)
	{
		glGenVertexArrays(1, &_vao);
		GLERR();
		glGenBuffers(1, &_vertexBuffer);
		glGenBuffers(1, &_indexBuffer);
		glBindVertexArray(_vao);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, vertSize, pvertices, GL_STATIC_DRAW);
		GLERR();
		size_t offset = 0;
		for (size_t i = 0; i < vertexAttributes.vertexAttribs.size();i++) {
			auto& attr = vertexAttributes.vertexAttribs[i];
			GLenum type = GL_FLOAT;
			size_t size = 0;
			size_t count = 0;
			switch (attr) {
			case Renderer::ShaderDataType::Float:
				count = 1;
				size = sizeof(float);
				type = GL_FLOAT;
				break;
			case Renderer::ShaderDataType::Float2:
				count = 2;
				size = sizeof(float);
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
			glVertexAttribPointer((GLuint)i,(GLint) count, type, GL_FALSE, vertexAttributes.vertexStride, (void*)(offset));
			GLERR();
			glEnableVertexAttribArray((GLuint)i);
			GLERR();
			offset += size*count;
		}
		GLERR();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, pindices, GL_STATIC_DRAW);
		GLERR();
	}
	uint32_t GLMultiMesh::GetPartCount() const
	{
		return (uint32_t)_primitives.size();
	}
	uint32_t GLMultiMesh::GetMaterialIndex(uint32_t part) const
	{
		return _primitives[part].materialIndex;
	}
	void GLMultiMesh::Bind()
	{
		glBindVertexArray(_vao);
		GLERR();

	}
	size_t GLMultiMesh::GetHash()
	{
		return _hash;
	}
	void GLMultiMesh::Render(uint32_t iPart) const
	{
		glDrawElementsBaseVertex(GL_TRIANGLES, _primitives[iPart].indexCount, GL_UNSIGNED_INT, (void*)(size_t)(_primitives[iPart].indexStart*sizeof(uint32_t)),_primitives[iPart].vertexStart);
	}
	void GLMultiMesh::GetBoundingBox(vec3& min, vec3& max)
	{
		min = _min;
		max = _max;
	}
	void GLMultiMesh::GetBoundingSphere(vec3& center, float& radius)
	{
		center = _center;
		radius = _radius;
	}
	void GLMultiMesh::GetWorldXForm(mat4& world)
	{
		world = _world;
	}
}