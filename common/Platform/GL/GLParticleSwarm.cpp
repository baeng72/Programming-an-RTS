#include "GLParticleSwarm.h"
#include "GLERR.h"

namespace GL {
	GLParticleSwarm::GLParticleSwarm(Renderer::RenderDevice* pdevice, Renderer::ParticleVertex* pvertices, uint32_t vertexCount, glm::vec2& size)
		:_size(size),_vao(-1),_vertexBuffer(-1)
	{
		_vertexCount = 0;
		const char* vertexSrc = R"(
#version 460 core

layout(location=0) in vec3 inPos;
layout(location=1) in vec4 inColor;


out vec3 aCenterW;
out vec4 aColor;


void main(){
	// Just pass data over to geometry shader.
	aCenterW = inPos;		
	aColor=inColor;
}

)";

		const char* geometrySrc = R"(
#version 460 core 
layout (points) in;
layout (triangle_strip,max_vertices=4) out;


layout(location=0) in vec3 aCenterW[];
layout(location=1) in vec4 aColor[];

out vec4 fColor;

uniform mat4 viewProj;
uniform vec3 eyePosW;
uniform vec2 sizeW;

void main(){
//
	// Compute the local coordinate system of the sprite relative to the world
	// space such that the billboard is aligned with the y-axis and faces the eye.
	//
	vec3 up = vec3(0.0f, 1.0f, 0.0f);
	
	vec3 look = eyePosW - aCenterW[0];
	look.y = 0.0f; // y-axis aligned, so project to xz-plane
	look = normalize(look);
	vec3 right = cross(up, look);
	
	//
	// Compute triangle strip vertices (quad) in world space.
	//
	float halfWidth  = 0.5f*sizeW.x;
	float halfHeight = 0.5f*sizeW.y;
	
	vec4 v[4];
	v[0] = vec4(aCenterW[0] + halfWidth*right - halfHeight*up, 1.0f);
	v[1] = vec4(aCenterW[0] + halfWidth*right + halfHeight*up, 1.0f);
	v[2] = vec4(aCenterW[0] - halfWidth*right - halfHeight*up, 1.0f);
	v[3] = vec4(aCenterW[0] - halfWidth*right + halfHeight*up, 1.0f);

	//
	// Transform quad vertices to world space and output 
	// them as a triangle strip.
	//
	for(int i=0;i<4;++i){
		gl_Layer=0;//one face?		
		gl_Position = viewProj*v[i];				
		fColor = aColor[0];
		EmitVertex();
	}
	EndPrimitive();    
}
)";
		const char* fragmentSrc = R"(
#version 460 core
layout(location=0) in vec4 fColor;
out vec4 aFragColor;

void main(){
	aFragColor = fColor;
}
)";
		_shader.compile(vertexSrc, geometrySrc, fragmentSrc);
		_shader.EnableCull(false);
		_shader.EnableBlend(false);
		_shader.EnableDepth(false);
		glGenVertexArrays(1, &_vao);
		
		uint32_t vertSize = vertexCount * sizeof(Renderer::ParticleVertex);

		glBindVertexArray(_vao);		
		glGenBuffers(1, &_vertexBuffer);		
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);		
		glBufferData(GL_ARRAY_BUFFER, vertSize, pvertices, GL_STATIC_DRAW);		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Renderer::ParticleVertex), 0);							//pos		
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer::ParticleVertex), (void*)sizeof(vec3));			//color		
		glEnableVertexAttribArray(0);		
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
		_vertexCount = vertexCount;
	}
	GLParticleSwarm::~GLParticleSwarm()
	{	
		if (_vertexBuffer > -1) {
			glDeleteBuffers(1, &_vertexBuffer);			
		}
		glDeleteVertexArrays(1, &_vao);		
	}
	void GLParticleSwarm::Draw(glm::mat4& viewProj, glm::vec3& eyePos)
	{
		if (_vertexCount) {
			
			glBindVertexArray(_vao);			
			glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
			_shader.Bind();
			_shader.setMat4("viewProj", viewProj);
			_shader.setVec3("eyePosW", eyePos);
			_shader.setVec2("sizeW", _size);
			
			//glFrontFace(GL_CCW);			
			glDrawArrays(GL_POINTS, 0, _vertexCount);	
			glBindVertexArray(0);
		}
	}
	void GLParticleSwarm::ResetVertices(Renderer::ParticleVertex* pvertices, uint32_t vertexCount)
	{
		if (_vertexBuffer > -1) {
			glDeleteBuffers(1, &_vertexBuffer);			
		}
		uint32_t vertSize = vertexCount * sizeof(Renderer::ParticleVertex);
		
		glBindVertexArray(_vao);		
		glGenBuffers(1, &_vertexBuffer);		
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);		
		glBufferData(GL_ARRAY_BUFFER, vertSize, pvertices, GL_STATIC_DRAW);		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Renderer::ParticleVertex), 0);							//pos	
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Renderer::ParticleVertex), (void*)sizeof(vec3));			//color
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);		
		_vertexCount = vertexCount;
		glBindVertexArray(0);
	}
}