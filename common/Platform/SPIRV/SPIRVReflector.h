#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "SPIRVTypes.h"
namespace SPIRVUtils {
	
	struct SPIRVShaderReflection {
		std::vector<std::vector<SPIRVBinding>> bindings;
		std::vector<std::tuple<std::string, SPIRVFormat, uint32_t>>inputs;
		
		SPIRVPushBlock pushBlock;
		std::vector<std::tuple<std::string, int, int, int, uint32_t, uint32_t, uint32_t, void*>>blockmembers;//flat list of all resources (name,parentrow,set,binding,offset,size,paddedsize,buffer)
		std::unordered_map<size_t, int>blockmap;//hash of name to row in blockmembers, hashed names could be fully qualified, e.g. UBO.light.diffuse; partially: light.diffuse or not qualified: diffuse
	};
	class SPIRVReflector {				
	public:
		SPIRVReflector();
		~SPIRVReflector();
		void reflect(std::unordered_map<SPIRVShaderStage, std::vector<uint32_t>>& spirvMap, SPIRVShaderReflection& reflection);
	};
}