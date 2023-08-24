#include "stb_image_impl.h"
#include <stb_image.h>
#include <stb_image_resize.h>
#include "../../Core/Log.h"

Renderer::Image* Renderer::Image::Create(const char* pfilename, int desiredwidth, int desiredheight) {
	return new STB::STBImageImpl(pfilename,desiredwidth,desiredheight);
}

namespace STB {
	STBImageImpl::STBImageImpl(const char* pfilename, int desiredwidth, int desiredheight) {
		_pixels = nullptr;
		_width = _height = _channels = 0;
		Load(pfilename, desiredwidth, desiredheight);
	}
	STBImageImpl::~STBImageImpl()
	{
		if(_pixels)
			stbi_image_free(_pixels);
	}
	void STBImageImpl::Load(const char* pfilename, int desiredwidth, int desiredheight)
	{
		LOG_INFO("Loading image: {0}.", pfilename);
		_pixels = stbi_load(pfilename, &_width, &_height, &_channels, 4);		
		ASSERT(_pixels, "Error loading image.");
		if (desiredwidth >0 && desiredwidth != _width || desiredheight > 0 && desiredheight != _height) {
			//need to resize, probably better ways to do this, but meh
			stbi_uc* newTexPixels = (stbi_uc*)malloc(desiredwidth * desiredheight * _channels);

			stbir_resize_uint8(_pixels,_width, _height, _width,
				newTexPixels, desiredwidth, desiredheight, 0,_channels);

			stbi_image_free(_pixels);
			_pixels = newTexPixels;
			_width = desiredwidth;
			_height = desiredheight;
		}
	}
	void* STBImageImpl::GetPixels()
	{
		return _pixels;
	}
	void STBImageImpl::GetSize(int& width, int& height, int& channels)
	{
		width = _width;
		height = _height;
		channels = _channels;
	}
}