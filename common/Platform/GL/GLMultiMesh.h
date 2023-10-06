#include "../../mesh/MultiMesh.h"
#include <glad/glad.h>

namespace GL {
	class GLMultiMesh : public Mesh::MultiMesh {
		Renderer::RenderDevice* _pdevice;
		GLuint _vao;
		GLuint _vertexBuffer;
		GLuint _indexBuffer;
		std::vector<Mesh::Primitive> _primitives;
		mat4 _world;
		vec3 _min;
		vec3 _max;
		vec3 _center;
		float _radius;
		size_t _hash;
		void Create(float* pvertices, uint32_t vertSize, uint32_t* pindices, uint32_t indSize, Renderer::VertexAttributes& vertexAttributes);
	public:
		GLMultiMesh(Renderer::RenderDevice* pdevice, mat4& xform, std::vector<float>& vertices, std::vector<uint32_t>& indices, std::vector<Mesh::Primitive>& primitives, Renderer::VertexAttributes& vertexAttributes);
		virtual ~GLMultiMesh();
		virtual uint32_t GetPartCount()const override;
		virtual uint32_t GetMaterialIndex(uint32_t part)const override;
		virtual void Bind() override;
		virtual size_t GetHash()override;
		virtual void Render(uint32_t iPart)const override;
		virtual void GetBoundingBox(vec3& min, vec3& max)override;
		virtual void GetBoundingSphere(vec3& center, float& radius)override;
		virtual void GetWorldXForm(mat4& world)override;
	};
}