#include "GLTexture.h"
#include "../stb/stb_image.h"

namespace GL {
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, const char* pfile,vec2 size)
		:_size(size)

	{
		_pdevice = pdevice;
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(pfile, &width, &height, &channels, STBI_rgb_alpha);
		glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		GLint format;
		if (channels == 1) {
			format = GL_RED;
		}
		else if (channels == 3) {
			format = GL_RGB;
		}
		else if (channels == 4) {
			format = GL_RGBA;
		}
		else {
			assert(0);//format?
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		_width = width;
		_height = height;
		_channels = channels;
		if (_size.x == -1 || _size.y == -1) {
			_size = glm::vec2(_width, _height);
		}
		tex.textureID = _textureID;
		tex.width = _width;
		tex.height = _height;
		stbi_image_free(data);
	}
	
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, int width, int height, int bytesperpixel, uint8_t* pixels):_size(glm::vec2(width, height))
	{
		_pdevice = pdevice;
		GLint format;
		switch (bytesperpixel) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
				break;
		case 4:
			format = GL_RGBA;
				break;
		default:
			assert(0);//format?
			break;
		}
		glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		_width = width;
		_height = height;
		_channels = bytesperpixel;
		tex.textureID = _textureID;
		tex.width = _width;
		tex.height = _height;
	}
	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &_textureID);
	}
	void* GLTexture::GetNativeHandle() const
	{
		
		
		return (void*)&tex;//might need more info?
	}
	vec2 GLTexture::GetScale() const
	{
		vec2 scale = glm::vec2(_size.x / (float)_width, _size.y / (float)_height);
		return scale;
	}
}