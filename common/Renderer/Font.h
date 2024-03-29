#pragma once
#include "../Core/defines.h"
#include "RenderDevice.h"

namespace Renderer {
	class Font {
	public:
		static Font* Create();
		virtual ~Font() = default;
		virtual void Init(RenderDevice* pdevice, const char* pfont, int fontSize) = 0;
		virtual void Draw(const char* ptext, int xpos, int ypos,Color color = Color(1.f)) = 0;
		
		virtual void Render() = 0;
		virtual void Clear() = 0;
		virtual void GetTextSize(const char* ptext, float& width, float& height) = 0;
		virtual void SetDimensions(int width, int height) = 0;
	};
}