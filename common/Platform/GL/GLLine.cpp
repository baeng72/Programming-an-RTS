#include "GLLine.h"

namespace GL {
	GLLine::GLLine(Renderer::RenderDevice* pdevice, Renderer::LineVertex* vertices, uint32_t vertexCount, float lineWidth, bool isLineList)
		:_pdevice(pdevice),_isLineList(isLineList)
	{
		const char* vertexSrc = R"(
#version 460 core

layout(location=0) in vec3 inPos;
layout(location=1) in vec4 inColor;

out vec4 aColor;


uniform	mat4 projection;


void main(){
	gl_Position = projection * vec4(inPos,1.0);
	aColor = inColor;
}
	
)";


		const char* fragmentSrc = R"(
#version 460 core
layout(location=0) in vec4 aColor;
out vec4 FragColor;

void main(){
	FragColor = aColor;
}
)";
		_shader.compile(vertexSrc, nullptr, fragmentSrc);

		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);
		glGenBuffers(1, &_vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		uint32_t vertSize = vertexCount * sizeof(Renderer::LineVertex);
		glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Renderer::LineVertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer::LineVertex), (void*)sizeof(vec3));		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		GLERR();
		_vertexCount = vertexCount;
	}
	GLLine::~GLLine() {
		glDeleteBuffers(1, &_vertexBuffer);
		glDeleteVertexArrays(1, &_vao);
	}
	void GLLine::Draw(glm::mat4& viewProj)
	{
		_shader.Bind();
		_shader.setMat4("projection", viewProj);
		glBindVertexArray(_vao);
		glDrawArrays(_isLineList ? GL_LINES : GL_LINE_STRIP,0,_vertexCount);
	}
	void GLLine::ResetVertices(Renderer::LineVertex* vertices, uint32_t vertexCount)
	{
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
		uint32_t vertSize = vertexCount * sizeof(Renderer::LineVertex);
		glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Renderer::LineVertex), 0);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer::LineVertex), (void*)sizeof(vec3));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		GLERR();
		_vertexCount = vertexCount;
	}
}