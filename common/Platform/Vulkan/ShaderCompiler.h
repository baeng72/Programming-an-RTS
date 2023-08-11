#pragma once
#include <vector>
#include <vulkan/vulkan.h>
class ShaderCompiler {
public:
	ShaderCompiler();
	~ShaderCompiler();
	std::vector<uint32_t> compileShader(const char* shaderSrc, VkShaderStageFlagBits shaderStage);
};
