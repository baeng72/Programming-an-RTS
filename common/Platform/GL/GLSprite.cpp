#include "GLSprite.h"

namespace GL {
	GLSprite::GLSprite(Renderer::RenderDevice* pdevice)
		:_pdevice(pdevice)
	{
		const char* vertexSrc = R"(
#version 460 
out vec2 aUV;

vec3 vertices[4]={
	{0.0,0.0,0.0},
	{ 1.0,0.0,0.0},
	{ 1.0, 1.0,0.0},
	{ 0.0, 1.0,0.0},
};

vec2 uvs[4]={
	{0.0,0.0},
	{1.0,0.0},
	{1.0,1.0},
	{0.0,1.0},
};

int indices[6] = {
	0,1,2,0,2,3
};

uniform mat4 projection;	
uniform mat4 model;


void main(){
	vec3 inPos = vertices[indices[gl_VertexID]];
	vec2 inUV = uvs[indices[gl_VertexID]];
	gl_Position = projection * model * vec4(inPos,1.0);
	aUV = inUV;
}

)";
		const char* fragmentSrc = R"(
#version 460 
layout (location=0) in vec2 aUV;
out vec4 outFragColor;

uniform sampler2D text;

void main(){
	vec4 sampled = texture(text,aUV);
	outFragColor = sampled;
}
)";
		_shader.compile(vertexSrc, nullptr, fragmentSrc);
		int width, height;
		pdevice->GetDimensions(&width, &height);
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__
		_orthoproj = vulkOrthoRH(0.f, (float)width, 0.f, (float)height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#else
		_orthoproj = glOrthoRH(0.f,(float)width,0.f, (float)height, -1.f, 1.f);// glm::ortho(0.f, (float)width, (float)height, (float)0.f, -1.f, 1.f);
#endif
		_xform = mat4(1.f);
		glGenVertexArrays(1, &_vao);
	}
	GLSprite::~GLSprite()
	{
		glDeleteVertexArrays(1, &_vao);
	}
	void GLSprite::Draw(Renderer::Texture* ptexture, vec3& position)
	{
		struct GLTextureInfo {
			int textureID;
			int width;
			int height;
		};
		
		GLTextureInfo* pText = (GLTextureInfo*)ptexture->GetNativeHandle();
		glm::mat4 id = glm::mat4(1.f);
		glm::mat4 t = glm::translate(_xform, position);
		glm::mat4 s = glm::scale(id, glm::vec3(pText->width, pText->height, 0.f));
		glm::mat4 model = t * s;
		mat4 mvp = _orthoproj * model;
		vec4 p = mvp * vec4(0.f, 0.f, 0.f, 1.f);
		glFrontFace(GL_CW);
		glBindVertexArray(_vao);
		_shader.Bind();
		_shader.setMat4("projection", _orthoproj);
		_shader.setMat4("model", model);
		glBindTexture(GL_TEXTURE_2D, pText->textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}