#include "GLTexture.h"
#include "../stb/stb_image.h"
#include "GLERR.h"
#include "stb/stb_image_write.h"
namespace GL {


	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, const char* pfile,vec2 size, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter)
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
			_fmt = Renderer::TextureFormat::R8;
		}
		else if (channels == 2) {
			format = GL_RG;
			_fmt = Renderer::TextureFormat::R8G8;
		}
		else if (channels == 3) {
			format = GL_RGB;
			_fmt = Renderer::TextureFormat::R8G8B8;
		}
		else if (channels == 4) {
			format = GL_RGBA;
			_fmt = Renderer::TextureFormat::R8G8B8A8;
		}
		else {
			assert(0);//format?
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		GLERR();
		_width = width;
		_height = height;
		_channels = channels;
		if (_size.x == -1 || _size.y == -1) {
			_size = glm::vec2(_width, _height);
		}
		tex.textureID = _textureID;
		tex.width = width;
		tex.height = height;
		tex.addrMode = GetSamplerAddressMode(samplerAdd);
		tex.filter = GetSamplerFilter(filter);
		tex.format = format;
		stbi_image_free(data);
	}
	
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, int width, int height,Renderer::TextureFormat fmt,uint8_t* pixels, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter)
		:_size(glm::vec2(width, height)), _fmt(fmt)
	{
		_pdevice = pdevice;
		GLint format;
		//_fmt = fmt;
		int bytesperpixel = 0;
		switch (fmt) {
		case Renderer::TextureFormat::R8:
			bytesperpixel = 1;
			format = GL_RED;
			break;
		case Renderer::TextureFormat::R8G8:
			format = GL_RG;
			bytesperpixel = 2;
			break;
		case Renderer::TextureFormat::R8G8B8:
			format = GL_RGB;
			bytesperpixel = 3;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			format = GL_RGBA;
			bytesperpixel = 4;
			break;
		case Renderer::TextureFormat::D32:
				bytesperpixel = 4;
				format = GL_DEPTH_COMPONENT32;
				break;
		default:
			assert(0);//format?
			break;
		}
		glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		GLERR();
		_width = width;
		_height = height;
		_channels = bytesperpixel;
		tex.textureID = _textureID;
		tex.width = width;
		tex.height = height;
		tex.addrMode = GetSamplerAddressMode(samplerAdd);
		tex.filter = GetSamplerFilter(filter);
		tex.format = format;
	}
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerAddress samplerAdd, Renderer::TextureSamplerFilter filter) 
		:_size(glm::vec2(width, height)),_fmt(fmt)
	{
		_pdevice = pdevice;
		GLint internalformat;
		int bytesperpixel = 0;
		GLuint gltype;
		GLenum glfmt;
		switch (fmt) {
		case Renderer::TextureFormat::R8:
			bytesperpixel = 1;
			internalformat = GL_RED;
			glfmt = GL_RED;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case Renderer::TextureFormat::R8G8:
			internalformat = GL_RG;
			glfmt = GL_RG;
			bytesperpixel = 2;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case Renderer::TextureFormat::R8G8B8:
			internalformat = GL_RGB;
			bytesperpixel = 3;
			glfmt = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			internalformat = GL_RGBA;
			glfmt = GL_RGBA;
			bytesperpixel = 4;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case Renderer::TextureFormat::D32:
			bytesperpixel = 4;
			internalformat = GL_DEPTH_COMPONENT;
			glfmt = GL_DEPTH_COMPONENT;
			gltype = GL_FLOAT;
			break;
		default:
			assert(0);//format?
			break;
		}
		glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, glfmt, gltype, nullptr);
		if(internalformat != GL_DEPTH_COMPONENT)
			glGenerateMipmap(GL_TEXTURE_2D);
		GLERR();
		_width = width;
		_height = height;
		_channels = bytesperpixel;
		tex.textureID = _textureID;
		tex.width = width;
		tex.height = height;
		tex.addrMode = GetSamplerAddressMode(samplerAdd);
		tex.filter = GetSamplerFilter(filter);
		tex.format = glfmt;
	}

	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, Renderer::Texture* psrc)
	{
		GLTexture* pglsrc = reinterpret_cast<GLTexture*>(psrc);
		GLTextureInfo* pinfo =(GLTextureInfo*) pglsrc->GetNativeHandle();
		_pdevice = pdevice;
		GLint internalformat;
		int bytesperpixel = 0;
		GLuint gltype;
		GLenum glfmt;
		switch (pinfo->format) {
		case GL_RED:
			_fmt = Renderer::TextureFormat::R8;
			bytesperpixel = 1;
			internalformat = GL_RED;
			glfmt = GL_RED;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case GL_RG:
			_fmt = Renderer::TextureFormat::R8G8;
			internalformat = GL_RG;
			glfmt = GL_RG;
			bytesperpixel = 2;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case GL_RGB:
			_fmt = Renderer::TextureFormat::R8G8B8;
			internalformat = GL_RGB;
			bytesperpixel = 3;
			glfmt = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case GL_RGBA:
			_fmt = Renderer::TextureFormat::R8G8B8A8;
			internalformat = GL_RGBA;
			glfmt = GL_RGBA;
			bytesperpixel = 4;
			gltype = GL_UNSIGNED_BYTE;
			break;
		case GL_DEPTH_COMPONENT:
			_fmt = Renderer::TextureFormat::D32;
			bytesperpixel = 4;
			internalformat = GL_DEPTH_COMPONENT;
			glfmt = GL_DEPTH_COMPONENT;
			gltype = GL_FLOAT;
			break;
		default:
			assert(0);//format?
			break;
		}
		glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, pinfo->width, pinfo->height, 0, glfmt, gltype, nullptr);
		if (internalformat != GL_DEPTH_COMPONENT)
			glGenerateMipmap(GL_TEXTURE_2D);
		GLERR();
		_width = pinfo->width;
		_height = pinfo->height;
		_channels = bytesperpixel;
		tex.textureID = _textureID;
		tex.width = pinfo->width;
		tex.height = pinfo->height;
		tex.addrMode = pinfo->addrMode;
		tex.filter = pinfo->filter;
		tex.format = glfmt;

		glCopyImageSubData(pinfo->textureID, GL_TEXTURE_2D, 0, 0,0,0,
			tex.textureID,GL_TEXTURE_2D,0,0,0,0,
			tex.width, tex.height, 1);
		GLERR();
		
		
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
	bool GLTexture::SaveToFile(const char* ppath)
	{
		int bytesperpixel = 0;
		GLenum format;
		switch (_fmt) {
		case Renderer::TextureFormat::R8:
			bytesperpixel = 1;
			format = GL_RED;
			break;
		case Renderer::TextureFormat::R8G8:
			format = GL_RG;
			bytesperpixel = 2;
			break;
		case Renderer::TextureFormat::R8G8B8:
			format = GL_RGB;
			bytesperpixel = 3;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			format = GL_RGBA;
			bytesperpixel = 4;
			break;
		case Renderer::TextureFormat::D32:
			bytesperpixel = 4;
			format = GL_DEPTH_COMPONENT32;
			break;
		default:
			assert(0);//format?
			break;
		}
		std::vector<uint8_t> pixels(_width * _height * bytesperpixel);
		GLint currID;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &currID);
		if(currID!=_textureID)
			glBindTexture(GL_TEXTURE_2D,_textureID);
		glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels.data());
		if (currID != _textureID)
			glBindTexture(GL_TEXTURE_2D, currID);
		stbi_write_jpg(ppath, _width, _height, bytesperpixel, pixels.data(), 100);
		return true;
	}

	GLenum GLTexture::GetSamplerAddressMode(Renderer::TextureSamplerAddress addrMode)
	{
		GLenum mode = GL_CLAMP_TO_EDGE;
		switch (addrMode) {
		case Renderer::TextureSamplerAddress::Clamp:
			mode = GL_CLAMP_TO_EDGE;
			break;
		case Renderer::TextureSamplerAddress::Border:
			mode = GL_CLAMP_TO_BORDER;
			break;
		case Renderer::TextureSamplerAddress::Repeat:
			mode = GL_REPEAT;
			break;
		}
		return mode;
	}
	GLenum GLTexture::GetSamplerFilter(Renderer::TextureSamplerFilter filt) {
		GLenum filter = GL_LINEAR;
		switch (filt) {
		case Renderer::TextureSamplerFilter::Linear:
			filter = GL_LINEAR;
			break;
		case Renderer::TextureSamplerFilter::Nearest:
			filter = GL_NEAREST;
			break;
		}
		return filter;
	}
}