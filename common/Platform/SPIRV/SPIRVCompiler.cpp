#include "SPIRVCompiler.h"
#include <iostream>
#include <cassert>
#include <sstream>
#include "../../Core/Log.h"
#include <glslang/SPIRV/GlslangToSpv.h>

#if defined NDEBUG
#pragma comment(lib,"../../../ThirdParty/glslang/SPIRV/Release/SPIRV.lib")//this sucks, not sure why cmake can't find it
#else
#pragma comment(lib,"../../../ThirdParty/glslang/SPIRV/DEBUG/SPIRVd.lib")//this sucks, not sure why cmake can't find it
#endif
namespace SPIRVUtils {
	//https://stackoverflow.com/questions/71422055/using-glslang-to-extract-all-uniform-delcarations-in-a-glsl-file
	class Traverser : public glslang::TIntermTraverser {
		std::unordered_map<std::string, std::tuple<std::string, int>>& resources;
	public:
		Traverser(std::unordered_map<std::string, std::tuple<std::string, int>>& r) :resources(r) {

		}
		
		virtual void visitSymbol(glslang::TIntermSymbol* symbol)override {
			auto& qualifier = symbol->getQualifier();
			auto name = symbol->getName();
			//std::cout << "Qualifer name: " << name << std::endl;
			if (qualifier.isUniformOrBuffer()) {
				int binding = 0;
				auto storage = glslang::GetStorageQualifierString(qualifier.storage);
				auto complete = symbol->getCompleteString();
				auto accessname = symbol->getAccessName();
				//std::cout << "Storage: " << storage << std::endl;

				if (qualifier.hasLocation()) {
					//std::cout << "Location: " << qualifier.layoutLocation << std::endl;
				}
				if (qualifier.hasBinding()) {
					//std::cout << "Binding: " << qualifier.layoutBinding << std::endl;
					binding = qualifier.layoutBinding;
					
				}
				if (qualifier.hasPacking()) {
					auto packing = glslang::TQualifier::getLayoutPackingString(qualifier.layoutPacking);
					//std::cout << "Packing: " << packing << std::endl;
				}
				resources[accessname.c_str()] = {storage, binding };
				
			}
		}
	};

	//https://lxjk.github.io/2020/03/10/Translate-GLSL-to-SPIRV-for-Vulkan-at-Runtime.html

	

	struct SpirvHelper
	{
		static void Init() {
			glslang::InitializeProcess();
		}

		static void Finalize() {
			glslang::FinalizeProcess();
		}

		static void InitResources(TBuiltInResource& Resources) {
			Resources.maxLights = 32;
			Resources.maxClipPlanes = 6;
			Resources.maxTextureUnits = 32;
			Resources.maxTextureCoords = 32;
			Resources.maxVertexAttribs = 64;
			Resources.maxVertexUniformComponents = 4096;
			Resources.maxVaryingFloats = 64;
			Resources.maxVertexTextureImageUnits = 32;
			Resources.maxCombinedTextureImageUnits = 80;
			Resources.maxTextureImageUnits = 32;
			Resources.maxFragmentUniformComponents = 4096;
			Resources.maxDrawBuffers = 32;
			Resources.maxVertexUniformVectors = 128;
			Resources.maxVaryingVectors = 8;
			Resources.maxFragmentUniformVectors = 16;
			Resources.maxVertexOutputVectors = 16;
			Resources.maxFragmentInputVectors = 15;
			Resources.minProgramTexelOffset = -8;
			Resources.maxProgramTexelOffset = 7;
			Resources.maxClipDistances = 8;
			Resources.maxComputeWorkGroupCountX = 65535;
			Resources.maxComputeWorkGroupCountY = 65535;
			Resources.maxComputeWorkGroupCountZ = 65535;
			Resources.maxComputeWorkGroupSizeX = 1024;
			Resources.maxComputeWorkGroupSizeY = 1024;
			Resources.maxComputeWorkGroupSizeZ = 64;
			Resources.maxComputeUniformComponents = 1024;
			Resources.maxComputeTextureImageUnits = 16;
			Resources.maxComputeImageUniforms = 8;
			Resources.maxComputeAtomicCounters = 8;
			Resources.maxComputeAtomicCounterBuffers = 1;
			Resources.maxVaryingComponents = 60;
			Resources.maxVertexOutputComponents = 64;
			Resources.maxGeometryInputComponents = 64;
			Resources.maxGeometryOutputComponents = 128;
			Resources.maxFragmentInputComponents = 128;
			Resources.maxImageUnits = 8;
			Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
			Resources.maxCombinedShaderOutputResources = 8;
			Resources.maxImageSamples = 0;
			Resources.maxVertexImageUniforms = 0;
			Resources.maxTessControlImageUniforms = 0;
			Resources.maxTessEvaluationImageUniforms = 0;
			Resources.maxGeometryImageUniforms = 0;
			Resources.maxFragmentImageUniforms = 8;
			Resources.maxCombinedImageUniforms = 8;
			Resources.maxGeometryTextureImageUnits = 16;
			Resources.maxGeometryOutputVertices = 256;
			Resources.maxGeometryTotalOutputComponents = 1024;
			Resources.maxGeometryUniformComponents = 1024;
			Resources.maxGeometryVaryingComponents = 64;
			Resources.maxTessControlInputComponents = 128;
			Resources.maxTessControlOutputComponents = 128;
			Resources.maxTessControlTextureImageUnits = 16;
			Resources.maxTessControlUniformComponents = 1024;
			Resources.maxTessControlTotalOutputComponents = 4096;
			Resources.maxTessEvaluationInputComponents = 128;
			Resources.maxTessEvaluationOutputComponents = 128;
			Resources.maxTessEvaluationTextureImageUnits = 16;
			Resources.maxTessEvaluationUniformComponents = 1024;
			Resources.maxTessPatchComponents = 120;
			Resources.maxPatchVertices = 32;
			Resources.maxTessGenLevel = 64;
			Resources.maxViewports = 16;
			Resources.maxVertexAtomicCounters = 0;
			Resources.maxTessControlAtomicCounters = 0;
			Resources.maxTessEvaluationAtomicCounters = 0;
			Resources.maxGeometryAtomicCounters = 0;
			Resources.maxFragmentAtomicCounters = 8;
			Resources.maxCombinedAtomicCounters = 8;
			Resources.maxAtomicCounterBindings = 1;
			Resources.maxVertexAtomicCounterBuffers = 0;
			Resources.maxTessControlAtomicCounterBuffers = 0;
			Resources.maxTessEvaluationAtomicCounterBuffers = 0;
			Resources.maxGeometryAtomicCounterBuffers = 0;
			Resources.maxFragmentAtomicCounterBuffers = 1;
			Resources.maxCombinedAtomicCounterBuffers = 1;
			Resources.maxAtomicCounterBufferSize = 16384;
			Resources.maxTransformFeedbackBuffers = 4;
			Resources.maxTransformFeedbackInterleavedComponents = 64;
			Resources.maxCullDistances = 8;
			Resources.maxCombinedClipAndCullDistances = 8;
			Resources.maxSamples = 4;
			Resources.maxMeshOutputVerticesNV = 256;
			Resources.maxMeshOutputPrimitivesNV = 512;
			Resources.maxMeshWorkGroupSizeX_NV = 32;
			Resources.maxMeshWorkGroupSizeY_NV = 1;
			Resources.maxMeshWorkGroupSizeZ_NV = 1;
			Resources.maxTaskWorkGroupSizeX_NV = 32;
			Resources.maxTaskWorkGroupSizeY_NV = 1;
			Resources.maxTaskWorkGroupSizeZ_NV = 1;
			Resources.maxMeshViewCountNV = 4;
			Resources.limits.nonInductiveForLoops = 1;
			Resources.limits.whileLoops = 1;
			Resources.limits.doWhileLoops = 1;
			Resources.limits.generalUniformIndexing = 1;
			Resources.limits.generalAttributeMatrixVectorIndexing = 1;
			Resources.limits.generalVaryingIndexing = 1;
			Resources.limits.generalSamplerIndexing = 1;
			Resources.limits.generalVariableIndexing = 1;
			Resources.limits.generalConstantMatrixVectorIndexing = 1;
		}

		static EShLanguage FindLanguage(const SPIRVShaderStage shaderStage) {
			switch (shaderStage) {
			case SPIRVShaderStage::Vertex:
				return EShLangVertex;
			case SPIRVShaderStage::TessellationControl:
				return EShLangTessControl;
			case SPIRVShaderStage::TessellationEvaluation:
				return EShLangTessEvaluation;
			case SPIRVShaderStage::Geometry:
				return EShLangGeometry;
			case SPIRVShaderStage::Fragment:
				return EShLangFragment;
			case SPIRVShaderStage::Compute:
				return EShLangCompute;						
			}
			return EShLangVertex;
		}

		static bool GLSLtoSPV(const SPIRVShaderStage shaderStage, const SPIRVSemantics semantics, const char* pshader, std::vector<unsigned int>& spirv/*, std::unordered_map<std::string, std::tuple<std::string, int>>& resources*/) {

			EShLanguage stage = FindLanguage(shaderStage);
			glslang::TShader shader(stage);
			glslang::TProgram program;
			const char* shaderStrings[1];
			TBuiltInResource Resources = {};
			InitResources(Resources);

			// Enable SPIR-V and Vulkan rules when parsing GLSL
			EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgBuiltinSymbolTable | EShMsgVulkanRules);
			glslang::EShClient client = glslang::EShClient::EShClientVulkan;
			glslang::EshTargetClientVersion version = glslang::EshTargetClientVersion::EShTargetVulkan_1_2;
			if (semantics == SPIRVSemantics::Vulkan) {
				messages = (EShMessages)(EShMsgSpvRules | EShMsgBuiltinSymbolTable | EShMsgVulkanRules);
				client = glslang::EShClient::EShClientVulkan;
			}
			else if (semantics == SPIRVSemantics::OpenGL) {
				messages = (EShMessages)(EShMsgSpvRules);
				client = glslang::EShClient::EShClientOpenGL;
				version = glslang::EShTargetOpenGL_450;
			}
			else {//hlsl
			}

			shaderStrings[0] = pshader;
			shader.setStrings(shaderStrings, 1);			
			
			shader.setEnvInput(glslang::EShSource::EShSourceGlsl, stage, client, 100);
			shader.setEnvClient(client, version);
			shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);
			bool result = false;
			if (semantics == SPIRVSemantics::OpenGL) {
				result = shader.parse(&Resources, 450, true, messages);
			}
			else {
				result = shader.parse(&Resources, 100, false, messages);
			}
			if (!result) {
				std::string info = shader.getInfoLog();
				if (info.substr(0, 6) == "ERROR:") {
					std::string remainder = info.substr(7, info.length() - 7);
					int lineNo = 0;
					int colNo = 0;
					std::string lineColStr = remainder.substr(0, remainder.find('\'') - 2);
					size_t pos = lineColStr.find(':');
					if (pos != std::string::npos) {
						std::string lineStr = lineColStr.substr(0, pos);
						std::string colStr = lineColStr.substr(pos + 1, lineColStr.length() - pos - 1);
						lineNo = std::atoi(lineStr.c_str());
						colNo = std::atoi(colStr.c_str());
						std::string errorLine = shader.getInfoDebugLog();
						LOG_WARN("Shader Compiler Error: {0}",info.c_str());
						int i = 0;
						std::stringstream str(pshader);
						std::string prevLine;
						std::string errLine;
						do {
							prevLine = errLine;
							std::getline(str, errLine);
							i++;
						} while (i < colNo);
						char buffer[256];
						if (colNo > 0) {
							sprintf_s(buffer, "Line %d: %s", colNo - 1, prevLine.c_str());
							LOG_WARN("Shader Compiler Error: {0}", buffer);
						}
						sprintf_s(buffer, "Line %d: %s", colNo, errLine.c_str());
						LOG_WARN("Shader Compiler Error: {0}", buffer);
					}


				}
				else
					LOG_WARN("Shader Compiler Error: {0}", shader.getInfoDebugLog());
					
				
				return false;  // something didn't work
			}
			/*resources.clear();
			Traverser traverser(resources);
			auto root = shader.getIntermediate()->getTreeRoot();
			root->traverse(&traverser);
			for (auto& pair : resources) {
				std::cout << "Resource: " << pair.first << ", type: " << std::get<0>(pair.second) << ", binding: " << std::get<1>(pair.second) << std::endl;
			}*/


			program.addShader(&shader);

			if (!program.link(messages)) {
				LOG_WARN("Shader compiler error: {0}", shader.getInfoDebugLog());
				return false;
			}
			
			glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
			return true;
		}
	};

	SPIRVCompiler::SPIRVCompiler() {
		SpirvHelper::Init();
	}

	SPIRVCompiler::~SPIRVCompiler() {
		SpirvHelper::Finalize();
	}

	std::vector<uint32_t> SPIRVCompiler::compileShader(const char* shaderSrc, SPIRVShaderStage shaderStage, SPIRVSemantics semantics) {
		std::vector<uint32_t> spirv;
		SpirvHelper::GLSLtoSPV(shaderStage, semantics, shaderSrc, spirv/*,resources*/);
		return spirv;
	}
}