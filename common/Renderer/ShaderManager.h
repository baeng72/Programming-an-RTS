#pragma once
#include <glm/glm.hpp>
#include "RenderDevice.h"
#include "Light.h"
#include "Texture.h"
namespace Renderer {
	class MeshShader;
	struct FlatShaderUBO {
		glm::mat4 viewProj;		
	};
	struct FlatShaderDirectionalUBO : public FlatShaderUBO {
		Renderer::DirectionalLight light;
		FlatShaderDirectionalUBO(glm::mat4& vp, Renderer::DirectionalLight& l) {
			viewProj = vp;
			light = l;
		}
	};
	struct FlatShaderPushConst {		
		glm::mat4 model;
	};
	class ShaderManager {
	public:
		enum ShaderType { Flat, FlatDirectional, MAX_SHADERS };
		static ShaderManager * Create(RenderDevice* pdevice);
		virtual ~ShaderManager() = default;
		virtual void* GetShaderData(ShaderType type,bool wireframe=false) = 0;
	};
}
