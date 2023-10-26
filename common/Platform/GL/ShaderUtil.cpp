#include "ShaderUtil.h"
#include "../../Core/Log.h"

namespace GL {
	ShaderUtil::ShaderUtil() {
		_programID = -1;
		_frontFace = GL_CW;
		_cullFace = GL_FRONT_FACE;
		_enableBlend = true;
	}
	GLuint ShaderUtil::compileShader(const char* shaderSrc, GLenum shaderType) {
		GLuint shaderID = -1;
		int success;
		char buffer[512];
		shaderID = glCreateShader(shaderType);
		glShaderSource(shaderID, 1, &shaderSrc, nullptr);
		glCompileShader(shaderID);
		//print errors, if any
		glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shaderID, 512, nullptr, buffer);
			LOG_ERROR("Unable to compile GL shader: {0}", buffer);
			assert(0);
		}
		return shaderID;
	}
	void ShaderUtil::compile(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc) {
		GLuint vertexID=-1, geometryID=-1, fragmentID=-1;
		char buffer[512];
		int success;
		GLERR();
		vertexID = compileShader(vertexSrc, GL_VERTEX_SHADER);
		if (geometrySrc && *geometrySrc)
			geometryID = compileShader(geometrySrc, GL_GEOMETRY_SHADER);
		
		fragmentID = compileShader(fragmentSrc, GL_FRAGMENT_SHADER);

		_programID = glCreateProgram();
		glAttachShader(_programID, vertexID);
		if (geometrySrc && *geometrySrc)
			glAttachShader(_programID, geometryID);
		glAttachShader(_programID, fragmentID);
		glLinkProgram(_programID);
		glGetProgramiv(_programID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(_programID, 512, nullptr, buffer);
			LOG_ERROR("Unable to link GL shader programe {0}", buffer);
			
		}
		glDeleteShader(vertexID);
		if (geometrySrc && *geometrySrc)
			glDeleteShader(geometryID);
		glDeleteShader(fragmentID);
		GLERR();
		{
			int count;
			GLenum type;
			GLint size;
			const int bufSize = 32;
			GLchar name[bufSize];
			GLsizei length;
			glGetProgramiv(_programID, GL_ACTIVE_UNIFORMS, &count);
			for (int i = 0; i < count; i++) {
				glGetActiveUniform(_programID, (GLuint)i, bufSize, &length, &size, &type, name);
				
				int location = glGetUniformLocation(_programID, name);
				size_t hash = Core::HashFNV1A(name, strlen(name));
				_uniformMap[hash] = location;
				if (type == GL_SAMPLER_2D) {
					int textureoffset = (int)_textureMap.size();
					_textureMap[hash] = textureoffset;
				}
			}
		}
		/*{
			int count;
			GLenum type;
			GLint size;
			const int bufSize = 32;
			GLchar name[bufSize];
			GLsizei length;
			glGetProgramiv(_programID, GL_ACTIVE_ATTRIBUTES, &count);
			for (int i = 0; i < count; i++) {
				glGetActiveAttrib(_programID, (GLuint)i, bufSize, &length, &size, &type, name);
				if (type == GL_FLOAT_VEC3) {

				}
			}
		}*/
	}
	
	ShaderUtil::ShaderUtil(const char* vertexSrc, const char* fragmentSrc)
	{
		compile(vertexSrc, nullptr, fragmentSrc);
	}
	ShaderUtil::ShaderUtil(const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc)
	{
		compile(vertexSrc, geometrySrc, fragmentSrc);
	}
	ShaderUtil::ShaderUtil(const char* fileName)
	{
		std::string filepath = fileName;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		std::string source = readFile(filepath);
		std::unordered_map<int, std::string> shaderSources = PreProcess(source);		
		compile(shaderSources[GL_VERTEX_SHADER].c_str(), shaderSources[GL_GEOMETRY_SHADER].c_str(), shaderSources[GL_FRAGMENT_SHADER].c_str());
	}
	ShaderUtil::~ShaderUtil()
	{
		glDeleteProgram(_programID);
	}
	std::string ShaderUtil::readFile(const std::string& filepath) {//borowed from TheCherno Hazel shader stuff
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		ASSERT(in, "Unable to open shader file.");
		in.seekg(0, std::ios::end);
		result.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&result[0], result.size());
		in.close();
		return result;
	}
	std::unordered_map<int, std::string> ShaderUtil::PreProcess(const std::string& source) {
		std::unordered_map<int, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			ASSERT(pos != std::string::npos, "Syntax error reading shader file.");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			unsigned int stage = ShaderTypeFromString(type);
			ASSERT(stage != (unsigned int)-1, "Invalid shader type specified!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[stage] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));

		}
		return shaderSources;
	}
	int ShaderUtil::ShaderTypeFromString(const std::string& type) {
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		else if (type == "geometry")
			return GL_GEOMETRY_SHADER;
		else if (type == "fragment")
			return GL_FRAGMENT_SHADER;
		return (int)-1;
	}
	void ShaderUtil::SetTexture(int texID) {
		glBindTexture(GL_TEXTURE_2D, texID);
		
		GLERR();
	}
	void ShaderUtil::SetTexture(const char*pname,int texID) {
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _uniformMap[hash];
		int offset = _textureMap[hash];
		glActiveTexture(GL_TEXTURE0 + offset);
		glBindTexture(GL_TEXTURE_2D, texID);
		
		glUniform1i(location, texID);
		GLERR();
	}
	void ShaderUtil::SetTextures(int* ptexids, uint32_t count) {
		//glUseProgram(_programID);
		for (uint32_t i = 0; i < count; i++) {
			glActiveTexture(i + GL_TEXTURE0);
			int texid = ptexids[i];
			glBindTexture(GL_TEXTURE_2D, texid);
		}
		glUniform1iv(0, count, (const GLint*)ptexids);
		GLERR();
	}
	void ShaderUtil::SetTextures(const char* pname, int* texids, uint32_t count) {
		size_t hash = Core::HashFNV1A(pname, strlen(pname));
		int location = _uniformMap[hash];
		//glUseProgram(_programID);
		std::vector<int> locs(count);
		int offset = _textureMap[hash];
		

		for (uint32_t i = 0; i < count; i++) {
			glActiveTexture(GL_TEXTURE0 + offset+ i);
			glBindTexture(GL_TEXTURE_2D, texids[i]);
			locs[i] = offset+i;
		}
		glUniform1iv(location, count, (const GLint*)locs.data());
		GLERR();
	}
	void ShaderUtil::SetStorageBuffer(GLuint buffer)
	{
		//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		//glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
		GLERR();
	}

	void ShaderUtil::Bind() {
		if (_enableDepth)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		GLERR();
		
		//glEnable(GL_CULL_FACE);
		//GLERR();
		if (_enableBlend) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
		}
		glDepthFunc(GL_LEQUAL);
		//glEnable(GL_CULL_FACE);
		//GLERR();
		//glCullFace(GL_BACK);
		//glFrontFace(GL_CW);
		//GLERR();
		glUseProgram(_programID);
		GLERR();
	}

	
}