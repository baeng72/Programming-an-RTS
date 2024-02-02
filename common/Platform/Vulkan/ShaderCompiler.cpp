#pragma once
#include "ShaderCompiler.h"
#include <cassert>
#include <sstream>
#include <glslang/SPIRV/GlslangToSpv.h>
//#if NDEBUG
//#pragma comment(lib,"glslang.lib")
//#pragma comment(lib,"OGLCompiler.lib")
//#pragma comment(lib,"MachineIndependent.lib")
//#pragma comment(lib,"OSDependent.lib")
//#pragma comment(lib,"GenericCodeGen.lib")
//#pragma comment(lib,"SPIRV.lib")
//#pragma comment(lib,"SPIRV-Tools-opt.lib")
//#pragma comment(lib,"SPIRV-Tools.lib")
//
//#else
//#pragma comment(lib,"glslangd.lib")
//#pragma comment(lib,"OGLCompilerd.lib")
//#pragma comment(lib,"MachineIndependentd.lib")
//#pragma comment(lib,"OSDependentd.lib")
//#pragma comment(lib,"GenericCodeGend.lib")
#if defined NDEBUG
#pragma comment(lib,"../../../ThirdParty/glslang/SPIRV/Release/SPIRV.lib")//this sucks, not sure why cmake can't find it
#else
#pragma comment(lib,"../../../ThirdParty/glslang/SPIRV/DEBUG/SPIRVd.lib")//this sucks, not sure why cmake can't find it
#endif
//#pragma comment(lib,"SPIRV-Tools-optd.lib")
//#pragma comment(lib,"SPIRV-Toolsd.lib")
//#endif
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

	static EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) {
		switch (shader_type) {
		case VK_SHADER_STAGE_VERTEX_BIT:
			return EShLangVertex;
		case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
			return EShLangTessControl;
		case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
			return EShLangTessEvaluation;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			return EShLangGeometry;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			return EShLangFragment;
		case VK_SHADER_STAGE_COMPUTE_BIT:
			return EShLangCompute;
		default:
			return EShLangVertex;
		}
	}

	static bool GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char* pshader, std::vector<unsigned int>& spirv) {

		EShLanguage stage = FindLanguage(shader_type);
		glslang::TShader shader(stage);
		glslang::TProgram program;
		const char* shaderStrings[1];
		TBuiltInResource Resources = {};
		InitResources(Resources);

		// Enable SPIR-V and Vulkan rules when parsing GLSL
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules | EShMsgBuiltinSymbolTable);

		shaderStrings[0] = pshader;
		shader.setStrings(shaderStrings, 1);
		shader.setEnvInput(glslang::EShSource::EShSourceGlsl, stage, glslang::EShClient::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClient::EShClientVulkan, glslang::EShTargetClientVersion::EShTargetVulkan_1_0);
		shader.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, glslang::EShTargetLanguageVersion::EShTargetSpv_1_0);
		if (!shader.parse(&Resources, 100, false, messages)) {
			std::string info = shader.getInfoLog();
			if (info.substr(0, 6) == "ERROR:") {
				std::string remainder = info.substr(7, info.length() - 7);
				int lineNo = 0;
				int colNo = 0;
				std::string lineColStr = remainder.substr(0,remainder.find('\'')-2);
				size_t pos = lineColStr.find(':');
				if (pos != std::string::npos) {
					std::string lineStr = lineColStr.substr(0, pos);
					std::string colStr = lineColStr.substr(pos + 1, lineColStr.length() - pos - 1);
					lineNo = std::atoi(lineStr.c_str());
					colNo = std::atoi(colStr.c_str());
					std::string errorLine = shader.getInfoDebugLog();
					puts(info.c_str());
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
						puts(buffer);
					}
					sprintf_s(buffer, "Line %d: %s", colNo, errLine.c_str());
					puts(buffer);
				}


			}else
				puts(shader.getInfoLog());
			//puts(shader.getInfoDebugLog());
			return false;  // something didn't work
		}


		/*class RefrectionTraverser : public glslang::TIntermTraverser {
		public:
			virtual void visitSymbol(glslang::TIntermSymbol* symbol) override {

				auto& qualifier = symbol->getQualifier();
				auto& name = symbol->getName();
				if (qualifier.isUniformOrBuffer()) {



					fprintf(stderr, "%s: %s\n", name.c_str(),
						glslang::GetStorageQualifierString(qualifier.storage));

					if (qualifier.hasLocation()) {
						fprintf(stderr, "  loc: %d\n", qualifier.layoutLocation);
					}
					if (qualifier.hasBinding()) {
						fprintf(stderr, "  binding: %d\n", qualifier.layoutBinding);
					}
					if (qualifier.hasPacking()) {
						fprintf(stderr, "  packing: %s\n",
							glslang::TQualifier::getLayoutPackingString(
								qualifier.layoutPacking));
					}
				}
			}

			virtual void visitConstantUnion(glslang::TIntermConstantUnion* constantUnion) override {
				auto& qualifier = constantUnion->getQualifier();

				bool isMatrix = constantUnion->isMatrix();
				if (isMatrix) {
					fprintf(stderr, "Matrix");
				}
			}
		};*/

		/*RefrectionTraverser traverser;

		auto root = shader.getIntermediate()->getTreeRoot();
		root->traverse(&traverser);*/
		program.addShader(&shader);

		//
		// Program-level processing...
		//

		if (!program.link(messages)) {
			//puts(shader.getInfoLog());
			puts(shader.getInfoDebugLog());
			fflush(stdout);
			return false;
		}
		/*struct Uniform {
			std::string name;
			uint32_t size{ 0 };
			uint32_t stages{ 0 };
			std::vector<Uniform> children;
		};
		if (program.buildReflection()) {
			int uniCount = program.getNumUniformBlocks();
			std::vector<Uniform> uniforms(uniCount);
			for (int i = 0; i < uniCount; i++) {
				auto val = program.getUniformBlock(i);
				uniforms[i].name = val.name;
				uniforms[i].size = val.size;
				uniforms[i].stages = val.stages;
				for (auto m = 0; m < val.numMembers; m++) {

				}
			}
		}*/

		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);
		return true;
	}
};


ShaderCompiler::ShaderCompiler()
{
	SpirvHelper::Init();
}
ShaderCompiler::~ShaderCompiler()
{
	SpirvHelper::Finalize();
}
std::vector<uint32_t> ShaderCompiler::compileShader(const char* shaderSrc, VkShaderStageFlagBits shaderStage)
{
	std::vector<uint32_t> spirv;
	SpirvHelper::GLSLtoSPV(shaderStage, shaderSrc, spirv);
	return spirv;
}
