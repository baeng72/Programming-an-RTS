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
		std::vector<GLint> _textureslots;
		GLenum _frontFace;
		GLenum _cullFace;
		bool _enableCull;
		bool _enableBlend;
		bool _enableDepth;
		GLenum _srcColor;
		GLenum _dstColor;
		GLenum _colorOp;
		GLenum _srcAlpha;
		GLenum _dstAlpha;
		GLenum _alphaOp;
	public:
		ShaderUtil();
		ShaderUtil(const ShaderUtil& rhs) = delete;
		ShaderUtil(const char* vertexSrc, const char* fragmentSrc);
		ShaderUtil(const char* vertexSrc, const char* geometrySrc,const char* fragmentSrc);
		ShaderUtil(const char* fileName);//load glsl file
		const ShaderUtil& operator=(const ShaderUtil& rhs);
		~ShaderUtil();
		void compile(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc);
		inline void ensureProgram()const {
			GLint id;
			glGetIntegerv(GL_CURRENT_PROGRAM, &id);
			if (id != _programID) {
				glUseProgram(_programID);
				GLERR();
			}
		}
		void setInt(int location, GLint v)const {
			ensureProgram();
			glUniform1i(location, v);
			GLERR();
		}
		void setInt(const char* name, GLint v)const {
			int location = glGetUniformLocation(_programID, name);
			GLERR();
			ensureProgram();
			glUniform1i(location, v);
			GLERR();
		}
		void setFloat(int location, float value)const {
			ensureProgram();
			glUniform1f(location, value);
		}
		void setFloat(const char* name, float value)const {
			int location = glGetUniformLocation(_programID, name);
			GLERR();
			ensureProgram();
			glUniform1f(location, value);
			GLERR();
		}
		
		void setVec2(unsigned int location, vec2& value)const {
			ensureProgram();
			glUniform2fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec2(const char* name, vec2& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			ensureProgram();
			GLERR();
			glUniform2fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec2(unsigned int location, vec2* pvalue)const {
			ensureProgram();
			glUniform2fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec2(const char* pname, vec2* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			ensureProgram();
			glUniform2fv(location, 1, (const GLfloat*)pvalue);
			GLERR();

		}
		void setVec3(unsigned int location, vec3& value)const {
			ensureProgram();
			glUniform3fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec3(const char * name, vec3& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			ensureProgram();
			glUniform3fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec3(unsigned int location, vec3* pvalue)const {
			ensureProgram();
			glUniform3fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec3(const char* pname, vec3* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			ensureProgram();
			glUniform3fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec4(unsigned int location, vec4& value)const {
			ensureProgram();
			glUniform4fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec4(const char* name, vec4& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			ensureProgram();
			glUniform4fv(location, 1, &value[0]);
			GLERR();
		}
		void setVec4(unsigned int location, vec4* pvalue)const {
			ensureProgram();
			glUniform4fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setVec4(const char* pname, vec4* pvalue) {
			size_t hash = Core::HashFNV1A(pname, strlen(pname));
			int location = _uniformMap[hash];
			ensureProgram();
			glUniform4fv(location, 1, (const GLfloat*)pvalue);
			GLERR();
		}
		void setMat4(unsigned int location, mat4& value)const {
			ensureProgram();
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
			GLERR();
		}
		void setMat4(const char* name, mat4& value) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			
			ensureProgram();
			glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
			GLERR();
		}
		void setMat4(unsigned int location, mat4* pvalue)const {
			ensureProgram();
			glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)pvalue);
			GLERR();
		}
		void setMat4(const char* name, mat4* pvalue) {
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = _uniformMap[hash];
			ensureProgram();
			glUniformMatrix4fv(location, 1, GL_FALSE, (const GLfloat*)pvalue);
			GLERR();
			
		}
		int GetUniformLocation(const char* name) {
			
			size_t hash = Core::HashFNV1A(name, strlen(name));
			int location = 0;
			if(_uniformMap.find(hash)!=_uniformMap.end())
				location =_uniformMap[hash];
			else {
				ensureProgram();
				location = glGetUniformLocation(_programID, name);
				GLERR();
				_uniformMap[hash] = location;
			}
			
			
			return location;
		}
		void SetTexture(int texID);
		void SetTexture(const char* pname, int texID);
		void SetTextures(int* ptexids, uint32_t count);
		void SetTextures(const char* pname, int* texids, uint32_t count);
		void SetSampler(GLenum addrMode, GLenum filter);
		void SetStorageBuffer(GLuint buffer);	
		void SetFrontFace(GLenum ff) { _frontFace = ff; }
		void SetCullFace(GLenum cf) { _cullFace = cf; _enableCull = true; }
		void EnableCull(bool enable) { _enableCull = enable; }
		void EnableBlend(bool blend) { _enableBlend = blend; }
		void SetBlendFunction(GLenum srcColor, GLenum dstColor, GLenum colorOp, GLenum srcAlpha, GLenum dstAlpha, GLenum alphaOp);
		void EnableDepth(bool depth) { _enableDepth = depth; }
		operator GLuint() { return _programID; }
		void* getProgramID()const { return (void*)&_programID; }
		void Bind();
	};
}