#pragma once

#include "RenderDevice.h"

namespace Renderer {
	class Image {
	public:
		static Image* Create(const char* pfilename,int desiredwidth=-1,int desiredheight=-1);
		virtual ~Image()=default;
		virtual void Load(const char* pfilename, int desiredwidth = -1, int desiredheight = -1) = 0;
		virtual void* GetPixels() = 0;
		virtual void GetSize(int& width, int& height, int& channels) = 0;
	};
}
