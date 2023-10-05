#pragma once
#include <glad/glad.h>
#include <string>
#include <unordered_map>
#include "../../Core/defines.h"
#include "../../Core/hash.h"
#include "GLERR.h"
namespace GL {
	class ShaderUtil {
		GLuint _programID;
		GLuint compileShader(const char* shaderSrc, GLenum shaderType);
		
		std::string readFile(const std::string& filepath);
		std::unordered_map<int, std::string> PreProcess(const std::string& src);
		int ShaderTypeFromString(const std::string& type);
		std::unordered_map<size_t, int> _uniformMap;
		std::unordered_map<size_t, int> _textureMap;//bad name
		GLenum _frontFace;
		GLenum _cullFace;
		bool _enableBlend;
		bool _enableDepth;
	public:
		ShaderUtil();
		ShaderUtil(const char* vertexSrc, const char* fragmentSrc);
		ShaderUtil(const char* vertexSrc, const char* geometrySrc,const char* fragmentSrc);
		ShaderUtil(const char* fileName);//load glsl file

		~ShaderUtil();
		void compile(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc);
		void setFloat(int location, float value)const {
			glUniform1f(location, value);
		}
		void setFloat(const char* name, float value)const {
			int location = glGetUniformLocation(_programID, name);
			GLERR();
			glUniform1f(location, value);
			GLERR();
		}
		
		void setVec2(unsigned int location, vec2& value)const {
			glUniform2fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec2(const char* name, vec2& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			glUniform2fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec2(unsigned int location, vec2* pvalue)const {
			glUniform2fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec2(const char* pname, vec2* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			glUniform2fv(location, 1, (const GLfloat*)pvalue);
			GLERR();

		}
		void setVec3(unsigned int location, vec3& value)const {
			glUniform3fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec3(const char * name, vec3& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			glUniform3fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec3(unsigned int location, vec3* pvalue)const {
			glUniform3fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec3(const char* pname, vec3* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			glUniform3fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec4(unsigned int location, vec4& value)const {
			glUniform4fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec4(const char* name, vec4& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			glUniform4fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec4(unsigned int location, vec4* pvalue)const {
			glUniform4fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec4(const char* pname, vec4* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			glUniform4fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setMat4(unsigned int location, mat4& value)const {
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
			GLERR();
		}
		void setMat4(const char* name, mat4& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
			GLERR();
		}
		void setMat4(unsigned int location, mat4* pvalue)const {
			GLERR();
			glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)pvalue);
			GLERR();
		}
		void setMat4(const char* name, mat4* pvalue) {
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)pvalue);
			GLERR();
			
		}
		int GetUniformLocation(const char* name) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			return location;
		}
		void SetTexture(int texID);
		void SetTexture(const char* pname, int texID);
		void SetTextures(int* ptexids, uint32_t count);
		void SetTextures(const char* pname, int* texids, uint32_t count);
		void SetStorageBuffer(GLuint buffer);	
		void SetFrontFace(GLenum ff) { _frontFace = ff; }
		void SetCullFace(GLenum cf) { _cullFace = cf; }
		void EnableBlend(bool blend) { _enableBlend = blend; }
		void EnableDepth(bool depth) { _enableDepth = depth; }
		operator GLuint() { return _programID; }
		void Bind();
	};
}