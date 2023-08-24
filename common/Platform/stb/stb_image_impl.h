#pragma once

#include "../../Renderer/Image.h"

namespace STB {
	class STBImageImpl : public Renderer::Image {
		uint8_t* _pixels;
		int _width;
		int _height;
		int _channels;
	public:
		STBImageImpl(const char* pfilename, int desiredwidth = -1, int desiredheight = -1);
		virtual ~STBImageImpl();
		virtual void Load(const char* pfilename, int desiredwidth = -1, int desiredheight = -1)override;
		virtual void* GetPixels()override;
		virtual void GetSize(int& width, int& height, int& channels)override;
	};
}
