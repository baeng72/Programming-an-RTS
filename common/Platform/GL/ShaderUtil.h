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
		int GetUniformLocation(const char* name) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			GLERR();
			return location;
		}
		operator GLuint() { return _programID; }
		void Bind() { 
			glUseProgram(_programID); 
			GLERR();
		}
	};
}