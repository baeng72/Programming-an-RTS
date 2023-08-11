#pragma once
#include "RenderDevice.h"
#include <glm/glm.hpp>
namespace Renderer {
	class Texture {
	public:
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile);
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size);
		virtual ~Texture() = default;		
		virtual void* GetNativeHandle()const = 0;
		virtual glm::vec2 GetScale()const = 0;
	};
}