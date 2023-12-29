#pragma once
#include "RenderDevice.h"
//#include <glm/glm.hpp>
namespace Renderer {
	class Texture {
	public:
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile);
		static Texture* Create(Renderer::RenderDevice* pdevice, const char* pfile,glm::vec2 size);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, int bytesperpixel, uint8_t* pixels);
		static Texture* Create(Renderer::RenderDevice* pdevice, int width, int height, int bytesperpixel);
		virtual ~Texture() = default;		
		virtual void* GetNativeHandle()const = 0;
		virtual glm::vec2 GetScale()const = 0;
		virtual void SetName(const char* pname) = 0;
		virtual bool SaveToFile(const char* ppath) = 0;
	};
}