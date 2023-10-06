#pragma once
#include "../../Renderer/Line.h"
#include <glad/glad.h>
#include "ShaderUtil.h"
namespace GL {
	class GLLine : public Renderer::Line {
		Renderer::RenderDevice* _pdevice;
		GLuint _vao;
		GLuint _vertexBuffer;
		int _vertexCount;
		ShaderUtil _shader;
		bool _isLineList;
	public:
		GLLine(Renderer::RenderDevice* pdevice, Renderer::LineVertex* vertices, uint32_t vertexCount, float lineWidth = 1, bool isLineList = false);
		virtual ~GLLine();
		virtual void Draw(glm::mat4& viewProj) override;
		virtual void ResetVertices(Renderer::LineVertex* vertices, uint32_t vertexCount) override;
	};
}