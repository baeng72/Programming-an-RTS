#include "GLShaderManager.h"
#include "../../Core/Log.h"
namespace GL {
	GLShaderManager::GLShaderManager(Renderer::RenderDevice* pdevice)
		:_pdevice(pdevice)
	{
	}
	GLShaderManager::~GLShaderManager()
	{
		_shaderList.clear();
	}
	void* GLShaderManager::GetShaderDataByName(const char* name)
	{
		ASSERT(_shaderList.find(name) != _shaderList.end(), "Unknown shader requested!");
		return _shaderList[name].get();
	}
	void* GLShaderManager::CreateShaderData(const char* shaderPath, Renderer::ShaderCullMode cullMode, bool enableBlend,bool enableDepth, Renderer::ShaderStorageType* ptypes, uint32_t numtypes, void* platformData)
	{
		Renderer::ShaderCreateInfo createInfo;
		createInfo.blendInfo.enable = enableBlend;
		createInfo.depthInfo.enable = enableDepth;
		createInfo.cullMode = cullMode;
		createInfo.ptypes = ptypes;
		createInfo.numtypes = numtypes;
		createInfo.platformData = platformData;
		return CreateShaderData(shaderPath, createInfo);
//		LOG_INFO("Loading GL shader: {0}", shaderPath);
//		std::string filepath = shaderPath;
//		auto lastSlash = filepath.find_last_of("/\\");
//		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
//		auto lastDot = filepath.rfind('.');
//		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
//		std::string name = filepath.substr(lastSlash, count);
//		if (_shaderList.find(name) == _shaderList.end()) {
//			std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(shaderPath);
//			//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);
//
//			shader->EnableCull(cullMode != Renderer::ShaderCullMode::None);
//#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
//			if (cullMode == Renderer::ShaderCullMode::frontFace)
//				shader->SetCullFace(GL_BACK);
//			else if (cullMode == Renderer::ShaderCullMode::backFace)
//				shader->SetCullFace(GL_FRONT);
//#else
//			if (cullMode == Renderer::ShaderCullMode::frontFace)
//				shader->SetCullFace(GL_FRONT);
//			else if (cullMode == Renderer::ShaderCullMode::backFace)
//				shader->SetCullFace(GL_BACK);
//#endif
//			else if (cullMode == Renderer::ShaderCullMode::frontandbackFace)
//				shader->SetCullFace(GL_FRONT_AND_BACK);
//			shader->EnableBlend(enableBlend);
//			shader->EnableDepth(enableDepth);
//			_shaderList[name] = std::move(shader);
//		}
//		return _shaderList[name].get();
	}
	GLenum GLShaderManager::GetBlendFactor(Renderer::ShaderBlendFactor factor) {
		GLenum res = 0;
		switch (factor) {
		case Renderer::ShaderBlendFactor::ConstantAlpha:
			res = GL_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::ConstantColor:
			res = GL_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::DstAlpha:
			res = GL_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::DstColor:
			res = GL_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::One:
			res = GL_ONE;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantAlpha:
			res = GL_ONE_MINUS_CONSTANT_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusConstantColor:
			res = GL_ONE_MINUS_CONSTANT_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstAlpha:
			res = GL_ONE_MINUS_DST_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusDstColor:
			res = GL_ONE_MINUS_DST_COLOR;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcAlpha:
			res = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::OneMinusSrcColor:
			res = GL_ONE_MINUS_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::SrcAlpha:
			res = GL_SRC_ALPHA;
			break;
		case Renderer::ShaderBlendFactor::SrcColor:
			res = GL_SRC_COLOR;
			break;
		case Renderer::ShaderBlendFactor::Zero:
			res = GL_ZERO;
			break;
		}
		return res;
	}
	GLenum GLShaderManager::GetBlendOp(Renderer::ShaderBlendOp op) {
		GLenum res = 0;
		switch (op) {
		case Renderer::ShaderBlendOp::Add:
			res = GL_FUNC_ADD;
			break;
		case Renderer::ShaderBlendOp::Max:
			res = GL_MAX;
			break;
		case Renderer::ShaderBlendOp::Min:
			res = GL_MIN;
			break;
		case Renderer::ShaderBlendOp::ReverseSubstract:
			res = GL_FUNC_REVERSE_SUBTRACT;
			assert(0);
			break;
		case Renderer::ShaderBlendOp::Subtract:
			res = GL_FUNC_SUBTRACT;
			break;
		}
		return res;
	}
	GLenum GLShaderManager::GetDepthCompare(Renderer::ShaderCompareOp op) {
		switch (op) {
		case Renderer::ShaderCompareOp::Always:
			return GL_ALWAYS;
		case Renderer::ShaderCompareOp::Equal:
			return GL_EQUAL;
		case Renderer::ShaderCompareOp::Greater:
			return GL_GREATER;
		case Renderer::ShaderCompareOp::GreaterOrEqual:
			return GL_GEQUAL;
		case Renderer::ShaderCompareOp::Less:
			return GL_LESS;
		case Renderer::ShaderCompareOp::LessOrEqual:
			return GL_LEQUAL;
		case Renderer::ShaderCompareOp::Never:
			return GL_NEVER;
		case Renderer::ShaderCompareOp::NotEqual:
			return GL_NOTEQUAL;
		}
		return GL_LESS;
	}
	void* GLShaderManager::CreateShaderData(const char* shaderPath, Renderer::ShaderCreateInfo&info)
	{
		LOG_INFO("Loading GL Shader: {0}", shaderPath);
		std::string filepath = shaderPath;
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		if (lastSlash > 0) {
			_path = filepath.substr(0, lastSlash);
		}
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		std::string name = filepath.substr(lastSlash, count);
		if (_shaderList2.find(name) == _shaderList2.end()) {
			std::string source = readFile(filepath);
			std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string> shaderSources = PreProcess(source);
			CompileShaderFile(name, shaderSources, info);

			
		}
		return (void*)_shaderList2[name].get();
//		if (_shaderList.find(name) == _shaderList.end()) {
//			std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(shaderPath);
//			//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);
//
//			shader->EnableCull(info.cullMode != Renderer::ShaderCullMode::None);
//#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
//			if (info.cullMode == Renderer::ShaderCullMode::frontFace)
//				shader->SetCullFace(GL_BACK);
//			else if (info.cullMode == Renderer::ShaderCullMode::backFace)
//				shader->SetCullFace(GL_FRONT);
//#else
//			if (info.cullMode == Renderer::ShaderCullMode::frontFace)
//				shader->SetCullFace(GL_FRONT);
//			else if (info.cullMode == Renderer::ShaderCullMode::backFace)
//				shader->SetCullFace(GL_BACK);
//#endif
//
//			else if (info.cullMode == Renderer::ShaderCullMode::frontandbackFace)
//				shader->SetCullFace(GL_FRONT_AND_BACK);
//			shader->EnableBlend(info.blendInfo.enable);
//			if (info.blendInfo.enable) {
//				shader->SetBlendFunction(GetBlendFactor(info.blendInfo.srcColorFactor), GetBlendFactor(info.blendInfo.dstColorFactor), GetBlendOp(info.blendInfo.colorOp), GetBlendFactor(info.blendInfo.srcAlphaFactor), GetBlendFactor(info.blendInfo.dstAlphaFactor), GetBlendOp(info.blendInfo.alphaOp));
//			}
//			shader->EnableDepth(info.depthInfo.enable);
//			
//			_shaderList[name] = std::move(shader);
//		}
//		return _shaderList[name].get();
	}

	void* GLShaderManager::CreateShaderData(const char* name, const char* vertexSrc, const char* geometrySrc, const char* fragmentSrc, Renderer::ShaderCreateInfo& info) {

		std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string> shaderSources;
		if (vertexSrc && *vertexSrc)
			shaderSources[SPIRVUtils::SPIRVShaderStage::Vertex] = vertexSrc;
		if (geometrySrc && *geometrySrc)
			shaderSources[SPIRVUtils::SPIRVShaderStage::Geometry] = geometrySrc;
		if (fragmentSrc && *fragmentSrc)
			shaderSources[SPIRVUtils::SPIRVShaderStage::Fragment] = fragmentSrc;
		CompileShaderFile(name, shaderSources, info);

		return (void*)_shaderList2[name].get();
//		if (_shaderList.find(name) != _shaderList.end()) {
//			_shaderList[name].reset();
//		}
//		std::unique_ptr<ShaderUtil> shader = std::make_unique<ShaderUtil>(vertexSrc,geometrySrc,fragmentSrc);
//		//shader->SetFrontFace(cullBackFaces ? GL_CW : GL_CCW);
//
//		shader->EnableCull(info.cullMode != Renderer::ShaderCullMode::None);
//#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
//		if (info.cullMode == Renderer::ShaderCullMode::frontFace)
//			shader->SetCullFace(GL_BACK);
//		else if (info.cullMode == Renderer::ShaderCullMode::backFace)
//			shader->SetCullFace(GL_FRONT);
//#else
//		if (info.cullMode == Renderer::ShaderCullMode::frontFace)
//			shader->SetCullFace(GL_FRONT);
//		else if (info.cullMode == Renderer::ShaderCullMode::backFace)
//			shader->SetCullFace(GL_BACK);
//#endif
//
//		else if (info.cullMode == Renderer::ShaderCullMode::frontandbackFace)
//			shader->SetCullFace(GL_FRONT_AND_BACK);
//		shader->EnableBlend(info.blendInfo.enable);
//		if (info.blendInfo.enable) {
//			shader->SetBlendFunction(GetBlendFactor(info.blendInfo.srcColorFactor), GetBlendFactor(info.blendInfo.dstColorFactor), GetBlendOp(info.blendInfo.colorOp), GetBlendFactor(info.blendInfo.srcAlphaFactor), GetBlendFactor(info.blendInfo.dstAlphaFactor), GetBlendOp(info.blendInfo.alphaOp));
//		}
//		shader->EnableDepth(info.depthInfo.enable);
//		
//		_shaderList[name] = std::move(shader);
//
//		return &_shaderList[name];
		
	}

	std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string> GLShaderManager::PreProcess(std::string& source) {
		
		std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string> shaderSources;
		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0);
		while (source.find("#include ") != source.npos) {
			const auto pos = source.find("#include ");
			const auto p1 = source.find('<', pos);
			const auto p2 = source.find('>', pos);
			if (p1 == source.npos || p2 == source.npos || p2 <= p1) {
				LOG_ERROR("Error with shader #include");
			}
			const std::string name = source.substr(p1 + 1, p2 - p1 - 1);
			const std::string include = readFile(_path + name);
			source.replace(pos, p2 - pos + 1, include.c_str());
		}
		while (pos != std::string::npos) {
			size_t eol = source.find_first_of("\r\n", pos);
			ASSERT(pos != std::string::npos, "Syntax error reading shader file.");
			size_t begin = pos + typeTokenLength + 1;
			std::string type = source.substr(begin, eol - begin);
			SPIRVUtils::SPIRVShaderStage stage = ShaderTypeFromString(type);
			ASSERT(stage != SPIRVUtils::SPIRVShaderStage::MaxEnum, "Invalid shader type specified!");

			size_t nextLinePos = source.find_first_of("\r\n", eol);
			pos = source.find(typeToken, nextLinePos);
			shaderSources[stage] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));

		}
		return shaderSources;
	}
	SPIRVUtils::SPIRVShaderStage GLShaderManager::ShaderTypeFromString(const std::string& type) {
		if (type == "vertex")
			return SPIRVUtils::SPIRVShaderStage::Vertex;
		else if (type == "geometry")
			return SPIRVUtils::SPIRVShaderStage::Geometry;
		else if (type == "fragment")
			return SPIRVUtils::SPIRVShaderStage::Fragment;
		else if (type == "compute")
			return SPIRVUtils::SPIRVShaderStage::Compute;
		return SPIRVUtils::SPIRVShaderStage::MaxEnum;
	}
	std::string GLShaderManager::readFile(const std::string& filepath) {//borowed from TheCherno Hazel shader stuff
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
	void GLShaderManager::CompileShaderFile(const std::string& shadername, std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::string>& shaderSources, Renderer::ShaderCreateInfo& createInfo) {
		SPIRVUtils::SPIRVCompiler compiler;
		std::unordered_map<SPIRVUtils::SPIRVShaderStage, std::vector<uint32_t>> spirvMap;
		for (auto& stage : shaderSources) {
			std::vector<uint32_t> spirv = compiler.compileShader(stage.second.c_str(), stage.first, SPIRVUtils::SPIRVSemantics::OpenGL);
			spirvMap[stage.first] = spirv;
		
		}
		std::unique_ptr<GLShaderData> shaderDataPtr = std::make_unique<GLShaderData>();
		GLShaderData& shaderData = *shaderDataPtr;
		
		SPIRVUtils::SPIRVReflector reflector;
		reflector.reflect(spirvMap, shaderData.reflection);
		GLuint vertexID = -1, geometryID = -1, fragmentID = -1;
		char buffer[512];
		int success;
		GLERR();
		const std::string& vertexSrc = shaderSources[SPIRVUtils::SPIRVShaderStage::Vertex].c_str();
		const std::string& geometrySrc = shaderSources[SPIRVUtils::SPIRVShaderStage::Geometry].c_str();
		const std::string& fragmentSrc = shaderSources[SPIRVUtils::SPIRVShaderStage::Fragment].c_str();
		vertexID = compileShader(vertexSrc.c_str(), GL_VERTEX_SHADER);
		if (geometrySrc.length()>0)
			geometryID = compileShader(geometrySrc.c_str(), GL_GEOMETRY_SHADER);

		fragmentID = compileShader(fragmentSrc.c_str(), GL_FRAGMENT_SHADER);

		shaderData.programID = glCreateProgram();
		glAttachShader(shaderData.programID, vertexID);
		if (geometrySrc.length())
			glAttachShader(shaderData.programID, geometryID);
		glAttachShader(shaderData.programID, fragmentID);
		glLinkProgram(shaderData.programID);
		GLERR();
		
		glGetProgramiv(shaderData.programID, GL_LINK_STATUS, &success);
		
		if (!success) {
			glGetProgramInfoLog(shaderData.programID, 512, nullptr, buffer);
			LOG_WARN("Unable to link GL shader program {0}", buffer);
			glDeleteProgram(shaderData.programID);
		}
		else {
			_shaderList2[shadername] = std::move(shaderDataPtr);
			int count;
			GLenum type;
			GLint size;
			const int bufSize = 32;
			GLchar fieldName[bufSize];
			GLsizei length;
			glGetProgramiv(shaderData.programID, GL_ACTIVE_UNIFORMS, &count);
			for (int i = 0; i < count; i++) {
				glGetActiveUniform(shaderData.programID, (GLuint)i, bufSize, &length, &size, &type, fieldName);

				int location = glGetUniformLocation(shaderData.programID, fieldName);
				GLenum props[1] = { GL_LOCATION };
				GLint res[1] = { 0 };
				length = 0;
				//glGetProgramResourceiv(_programID, GL_LOCATION, i, 1, props, 1, &length, res);
				size_t hash = Core::HashFNV1A(fieldName, strlen(fieldName));
				shaderData.uniformLocationMap[hash] = location;
				if (type == GL_SAMPLER_2D) {
					int textureoffset = (int)shaderData.texOffsetMap.size();
					shaderData.texOffsetMap[hash] = textureoffset;
					shaderData.texSlots.push_back(location);
					std::string sname = fieldName;
					size_t pos = sname.find('[');
					if (pos != std::string::npos) {
						fieldName[pos] = 0;
						hash = Core::HashFNV1A(fieldName, strlen(fieldName));
						shaderData.uniformLocationMap[hash] = location;
						shaderData.texOffsetMap[hash] = textureoffset;
					}

				}
				else if (type == GL_SAMPLER_CUBE) {
					int textureoffset = (int)shaderData.texOffsetMap.size();
					shaderData.texOffsetMap[hash] = textureoffset;
					shaderData.texSlots.push_back(location);
					std::string sname = fieldName;
					props[0] = GL_BUFFER_BINDING;
					res[0] = 0;
					length = 0;
					GLint val = 0;
					glGetIntegerv(GL_TEXTURE_CUBE_MAP, &val);
					size_t pos = sname.find('[');
					if (pos != std::string::npos) {
						fieldName[pos] = 0;
						hash = Core::HashFNV1A(fieldName, strlen(fieldName));
						shaderData.uniformLocationMap[hash] = location;
						shaderData.texOffsetMap[hash] = textureoffset;
					}
				}
			}
			int set = 0;//should not be more than 1 set in GL shader, but...
			for (auto& bindingSet : shaderData.reflection.bindings) {

				for (auto& binding : bindingSet) {
					size_t hash = Core::HashFNV1A(binding.name.c_str(),binding.name.length());
					int bindx = binding.binding;
					if (shaderData.uniformLocationMap.find(hash) != shaderData.uniformLocationMap.end()) {
						shaderData.uniformLocationMap[hash] = bindx;
					}
					switch (binding.descriptorType) {
					case SPIRVUtils::SPIRVDescriptorType::UniformBuffer:

						if (shaderData.resourceLocationMap.find(hash) == shaderData.resourceLocationMap.end()) {
							shaderData.resourceLocationMap[hash] = bindx;
						}
						else {
							shaderData.resourceLocationMap[hash] = bindx;
						}
						break;
					case SPIRVUtils::SPIRVDescriptorType::StorageBuffer:
						if (shaderData.resourceLocationMap.find(hash) == shaderData.resourceLocationMap.end()) {
							shaderData.resourceLocationMap[hash] = bindx;
						}
						else {
							shaderData.resourceLocationMap[hash] = bindx;
						}
						break;

					case SPIRVUtils::SPIRVDescriptorType::CombinedImageSampler:
						if (shaderData.resourceLocationMap.find(hash) == shaderData.resourceLocationMap.end()) {
							shaderData.resourceLocationMap[hash] = bindx;
						}
						else {
							shaderData.resourceLocationMap[hash] = bindx;
						}
					}
				}
				set++;
			}

			//shader settings
			{
				shaderData.enableCull = createInfo.cullMode != Renderer::ShaderCullMode::None;				
#if defined __GL__TOP__LEFT__ && defined __GL__ZERO__TO__ONE__ //flip clip control converts CCW to CW or vice versa
				if (createInfo.cullMode == Renderer::ShaderCullMode::frontFace)					
					shaderData.cullFace = GL_BACK;
				else if (createInfo.cullMode == Renderer::ShaderCullMode::backFace)					
					shaderData.cullFace = GL_FRONT;
#else
				if (createInfo.cullMode == Renderer::ShaderCullMode::frontFace)
					shaderData.cullFace = GL_FRONT;
				else if (createInfo.cullMode == Renderer::ShaderCullMode::backFace)
					shaderInfo.cullFace = GL_BACK;
#endif

				else if (createInfo.cullMode == Renderer::ShaderCullMode::frontandbackFace)					
					shaderData.cullFace = GL_FRONT_AND_BACK;				
				shaderData.enableBlend = createInfo.blendInfo.enable;
				if (createInfo.blendInfo.enable) {
					shaderData.srcColor = GetBlendFactor(createInfo.blendInfo.srcColorFactor);
					shaderData.dstColor = GetBlendFactor(createInfo.blendInfo.dstColorFactor);
					shaderData.colorOp = GetBlendOp(createInfo.blendInfo.colorOp);
					shaderData.srcAlpha = GetBlendFactor(createInfo.blendInfo.srcAlphaFactor);
					shaderData.dstAlpha = GetBlendFactor(createInfo.blendInfo.dstAlphaFactor);
					shaderData.alphaOp = GetBlendOp(createInfo.blendInfo.alphaOp);					
				}				
				shaderData.enableDepth = createInfo.depthInfo.enable;
				shaderData.depthCompare = GetDepthCompare(createInfo.depthInfo.compare);

			}
		}
		glDeleteShader(vertexID);
		if (geometrySrc.length() > 0)
			glDeleteShader(geometryID);
		glDeleteShader(fragmentID);
	}

	GLuint GLShaderManager::compileShader(const char* shaderSrc, GLenum shaderType) {

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
}