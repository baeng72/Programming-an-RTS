#pragma once 
#include <vector>
namespace Renderer {
	//type of mesh vertex buffer attribute
	enum class ShaderDataType { Float, Float2, Float3, Float4, Float3x3, Float4x4 ,Int, Int2,Int3,Int4};
	struct VertexAttributes {
		std::vector<ShaderDataType> vertexAttribs;
		uint32_t vertexStride;
	};
}