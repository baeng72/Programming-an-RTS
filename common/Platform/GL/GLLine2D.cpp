#include "GLLine2D.h"
#include "../../Core/Log.h"

namespace GL {
	GLLine2D::GLLine2D(Renderer::RenderDevice* pdevice)
		:_pdevice(pdevice)
	{
		const char* vertexSrc = R"(
#version 460 core

layout (location=0) out vec4 aColor;

uniform	mat4 projection;
uniform	vec2 start;
uniform	vec2 end;
uniform	vec4 color;


void main(){	
	aColor=color;
	vec2 verts[2]={start,end};
	gl_Position = projection * vec4(verts[gl_VertexID],0.0,1.0);
	
}

)";


		const char* fragmentSrc = R"(
#version 460 core

layout (location=0) in vec4 aColor;

out vec4 FragColor;

void main(){	
	FragColor = aColor;	
}
)";
		_shader.compile(vertexSrc, nullptr, fragmentSrc);
		glGenVertexArrays(1, &_vao);
	}
	GLLine2D::~GLLine2D()
	{
		glDeleteVertexArrays(1, &_vao);
	}
	void GLLine2D::Draw(vec2* pvertices, uint32_t count, vec4 color, float width)
	{
		if (count < 2)
			return;
		glBindVertexArray(_vao);
		glLineWidth(width);
		_shader.Bind();
		_shader.setMat4("projection", projection);
		_shader.setVec4("color", color)	;
		for (uint32_t i = 1; i < count; i++) {
			vec2 start = pvertices[i - 1];
			start.y = _height - start.y;
			vec2 end = pvertices[i];
			end.y = _height - end.y;
			_shader.setVec2("start",pvertices[i - 1]);
			_shader.setVec2("end",pvertices[i]);
			glDrawArrays(GL_LINES, 0, 2);
		}

	}
	void GLLine2D::Update(int width, int height)
	{
		_width = width;
		_height = height;

		projection = glOrthoRH(0.f, (float)width, 0.f,(float)height, 1.f, -1.f);
		
	}
}