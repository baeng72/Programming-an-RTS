#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "SPIRVTypes.h"

namespace SPIRVUtils {	

	
	

	class SPIRVCompiler {
		//std::unordered_map<std::string, std::tuple<std::string, int>> resources;
	public:
		SPIRVCompiler();
		~SPIRVCompiler();
		std::vector<uint32_t> compileShader(const char* shaderSrc, SPIRVShaderStage, SPIRVSemantics semantics = SPIRVSemantics::Vulkan);
		//const std::unordered_map<std::string, std::tuple<std::string, int>>& getResources()const { return resources; }
		
	};
}