#include "GLTexture.h"
#include "../stb/stb_image.h"
#include "GLERR.h"
#include "stb/stb_image_write.h"
namespace GL {


	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, const char* pfile,vec2 size, Renderer::TextureSamplerWrap wrap, Renderer::TextureSamplerFilter filter)
		:_size(size)

	{
		_pdevice = pdevice;
		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		auto data = stbi_load(pfile, &width, &height, &channels, STBI_rgb_alpha);
		_fmt = Renderer::TextureFormat::R8G8B8A8;
		/*glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);*/
		//GLint format;
		//if (channels == 1) {
		//	format = GL_RED;
		//	_fmt = Renderer::TextureFormat::R8;
		//}
		//else if (channels == 2) {
		//	format = GL_RG;
		//	_fmt = Renderer::TextureFormat::R8G8;
		//}
		//else if (channels == 3) {
		//	format = GL_RGB;
		//	_fmt = Renderer::TextureFormat::R8G8B8;
		//}
		//else if (channels == 4) {
		//	format = GL_RGBA;
		//	_fmt = Renderer::TextureFormat::R8G8B8A8;
		//}
		//else {
		//	assert(0);//format?
		//}
		Renderer::TextureCreateInfo ci;
		ci.width = width;
		ci.height = height;
		ci.format = _fmt;
		ci.filter = filter;
		ci.wrap = wrap;
		ci.pixels = data;
		create(pdevice, ci);
		if (size.x == -1 || size.y == -1) {
			size = vec2(ci.width, ci.height);
		}
		/*glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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
		tex.wrapMode = GetSamplerAddressMode(samplerAdd);
		tex.filter = GetSamplerFilter(filter);
		tex.format = format;
		tex.type = GL_TEXTURE_2D;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, tex.wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.filter);
		glBindTexture(GL_TEXTURE_2D, 0);*/
		stbi_image_free(data);
	}
	
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, int width, int height,Renderer::TextureFormat fmt,uint8_t* pixels, Renderer::TextureSamplerWrap wrap, Renderer::TextureSamplerFilter filter)
		:_size(glm::vec2(width, height)), _fmt(fmt)
	{
		_pdevice = pdevice;
		Renderer::TextureCreateInfo ci;
		ci.width = width;
		ci.height = height;
		ci.format = fmt;
		ci.pixels = pixels;
		ci.wrap = wrap;
		ci.filter = filter;
		create(pdevice, ci);
		//GLint format;
		////_fmt = fmt;
		//int bytesperpixel = 0;
		//GLenum type = GL_UNSIGNED_BYTE;
		//switch (fmt) {
		//case Renderer::TextureFormat::R8:
		//	bytesperpixel = 1;
		//	format = GL_RED;
		//	break;
		//case Renderer::TextureFormat::R8G8:
		//	format = GL_RG;
		//	bytesperpixel = 2;
		//	break;
		//case Renderer::TextureFormat::R8G8B8:
		//	format = GL_RGB;
		//	bytesperpixel = 3;
		//	break;
		//case Renderer::TextureFormat::R8G8B8A8:
		//	format = GL_RGBA;
		//	bytesperpixel = 4;
		//	break;
		//case Renderer::TextureFormat::D32:
		//		bytesperpixel = 4;
		//		format = GL_DEPTH_COMPONENT32;
		//		break;
		//case Renderer::TextureFormat::FR32G32B32:
		//	bytesperpixel = 4 * 3;
		//	format = GL_RGB;
		//	type = GL_FLOAT;
		//	break;
		//	
		//default:
		//	assert(0);//format?
		//	break;
		//}
		//glGenTextures(1, &_textureID);		
		//glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, pixels);
		//glGenerateMipmap(GL_TEXTURE_2D);
		//
		//GLERR();
		//_width = width;
		//_height = height;
		//_channels = bytesperpixel;
		//tex.textureID = _textureID;
		//tex.width = width;
		//tex.height = height;
		//tex.wrapMode = GetSamplerAddressMode(samplerAdd);
		//tex.filter = GetSamplerFilter(filter);
		//tex.format = format;
		//tex.type = GL_TEXTURE_2D;
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.filter);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.filter);
		//glGenerateMipmap(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, int width, int height, Renderer::TextureFormat fmt, Renderer::TextureSamplerWrap wrap, Renderer::TextureSamplerFilter filter) 
		:_size(glm::vec2(width, height)),_fmt(fmt)
	{
		
		_pdevice = pdevice;
		Renderer::TextureCreateInfo ci;
		ci.width = width;
		ci.height = height;
		ci.format = fmt;
		ci.filter = filter;
		ci.wrap = wrap;
		create(pdevice, ci);

		//GLint internalformat;
		//int bytesperpixel = 0;
		//GLuint gltype;
		//GLenum glfmt;
		//switch (fmt) {
		//case Renderer::TextureFormat::R8:
		//	bytesperpixel = 1;
		//	internalformat = GL_RED;
		//	glfmt = GL_RED;
		//	gltype = GL_UNSIGNED_BYTE;
		//	break;
		//case Renderer::TextureFormat::R8G8:
		//	internalformat = GL_RG;
		//	glfmt = GL_RG;
		//	bytesperpixel = 2;
		//	gltype = GL_UNSIGNED_BYTE;
		//	break;
		//case Renderer::TextureFormat::R8G8B8:
		//	internalformat = GL_RGB;
		//	bytesperpixel = 3;
		//	glfmt = GL_RGB;
		//	gltype = GL_UNSIGNED_BYTE;
		//	break;
		//case Renderer::TextureFormat::R8G8B8A8:
		//	internalformat = GL_RGBA;
		//	glfmt = GL_RGBA;
		//	bytesperpixel = 4;
		//	gltype = GL_UNSIGNED_BYTE;
		//	break;
		//case Renderer::TextureFormat::D32:
		//	bytesperpixel = 4;
		//	internalformat = GL_DEPTH_COMPONENT;
		//	glfmt = GL_DEPTH_COMPONENT;
		//	gltype = GL_FLOAT;
		//	break;
		//
		//default:
		//	assert(0);//format?
		//	break;
		//}
		//glGenTextures(1, &_textureID);
		//glBindTexture(GL_TEXTURE_2D, _textureID);
		//glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, glfmt, gltype, nullptr);
		//if(internalformat != GL_DEPTH_COMPONENT)
		//	glGenerateMipmap(GL_TEXTURE_2D);
		//GLERR();
		//_width = width;
		//_height = height;
		//_channels = bytesperpixel;
		//tex.textureID = _textureID;
		//tex.width = width;
		//tex.height = height;
		//tex.wrapMode = GetSamplerAddressMode(wrap);
		//tex.filter = GetSamplerFilter(filter);
		//tex.format = glfmt;
		//tex.type = GL_TEXTURE_2D;
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, tex.wrapMode);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.filter);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.filter);
		//glGenerateMipmap(GL_TEXTURE_2D);
		//glBindTexture(GL_TEXTURE_2D, 0);
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
		Renderer::TextureCreateInfo ci;
		ci.width = pinfo->width;
		ci.height = pinfo->height;
		ci.format = _fmt;
		create(pdevice, ci);
		_size = vec2(pinfo->width, pinfo->height);
		/*glGenTextures(1, &_textureID);
		glBindTexture(GL_TEXTURE_2D, _textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalformat, pinfo->width, pinfo->height, 0, glfmt, gltype, nullptr);
		if (internalformat != GL_DEPTH_COMPONENT)
			glGenerateMipmap(GL_TEXTURE_2D);
		GLERR();*/
		/*_width = pinfo->width;
		_height = pinfo->height;
		_channels = bytesperpixel;*/
		tex.textureID = _textureID;
		tex.width = pinfo->width;
		tex.height = pinfo->height;
		tex.wrapMode = pinfo->wrapMode;
		tex.filter = pinfo->filter;
		tex.format = glfmt;
		tex.type = pinfo->type;
		glCopyImageSubData(pinfo->textureID, GL_TEXTURE_2D, 0, 0,0,0,
			tex.textureID,GL_TEXTURE_2D,0,0,0,0,
			tex.width, tex.height, 1);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_S, tex.wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_T, tex.wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_R, tex.wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(_textureID, GL_TEXTURE_MAX_LEVEL, ci.mipMaps ? 1000 : 0);
		glTextureParameteri(_textureID, GL_TEXTURE_MIN_FILTER, tex.filter);
		glTextureParameteri(_textureID, GL_TEXTURE_MAG_FILTER, tex.filter);
		//glBindTexture(GL_TEXTURE_2D, 0);
		GLERR();
		
		
	}
	GLTexture::GLTexture(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci) {
		_pdevice = pdevice;
		create(pdevice, ci);
		_size = vec2(ci.width, ci.height);
		
		
	}

	GLTexture::~GLTexture()
	{
		glDeleteTextures(1, &_textureID);
	}

	void GLTexture::create(Renderer::RenderDevice* pdevice, Renderer::TextureCreateInfo& ci) {
		GLint internalformat;
		int bytesperchannel = 0;
		int channels = 0;

		GLenum glfmt;
		GLenum gltype = GL_UNSIGNED_BYTE;
		switch (ci.format) {
		case Renderer::TextureFormat::R8:
			bytesperchannel = 1;
			internalformat = GL_R8;
			glfmt = GL_RED;
			gltype = GL_UNSIGNED_BYTE;
			channels = 1;
			break;
		case Renderer::TextureFormat::R8G8:
			internalformat = GL_RG8;
			glfmt = GL_RG;
			bytesperchannel = 2;
			gltype = GL_UNSIGNED_BYTE;
			channels = 2;
			break;
		case Renderer::TextureFormat::R8G8B8:
			internalformat = GL_RGB8;
			bytesperchannel = 3;
			glfmt = GL_RGB;
			gltype = GL_UNSIGNED_BYTE;
			channels = 3;
			break;
		case Renderer::TextureFormat::R8G8B8A8:
			internalformat = GL_RGBA8;
			glfmt = GL_RGBA;
			bytesperchannel = 4;
			gltype = GL_UNSIGNED_BYTE;
			channels = 4;
			break;
		case Renderer::TextureFormat::D32:
			bytesperchannel = 4;
			internalformat = GL_DEPTH_COMPONENT32F;
			glfmt = GL_DEPTH_COMPONENT;
			gltype = GL_FLOAT;
			channels = 1;
			break;
		case Renderer::TextureFormat::FR32G32B32:
			bytesperchannel = 4;
			internalformat = GL_RGB32F;
			glfmt = GL_RGB;
			gltype = GL_FLOAT;
			channels = 3;
			break;
		case Renderer::TextureFormat::FR32G32B32A32:
			bytesperchannel = 4;
			internalformat = GL_RGBA32F;
			glfmt = GL_RGBA;
			gltype = GL_FLOAT;
			channels = 4;
			break;
		case Renderer::TextureFormat::FR16G16B16A16:
			bytesperchannel = 2;
			internalformat = GL_RGBA16F;
			glfmt = GL_RGBA;
			gltype = GL_FLOAT;
			channels = 4;
			break;
		default:
			assert(0);//format?
			break;
		}
		GLenum wrapMode = GL_CLAMP_TO_EDGE;
		switch (ci.wrap) {
		case Renderer::TextureSamplerWrap::Clamp:
			wrapMode = GL_CLAMP_TO_EDGE;
			break;
		case Renderer::TextureSamplerWrap::Border:
			wrapMode = GL_CLAMP_TO_BORDER;
			break;
		case Renderer::TextureSamplerWrap::Repeat:
			wrapMode = GL_REPEAT;
			break;
		default:
			assert(0);
			break;
		}
		GLenum filter = GL_LINEAR;
		switch (ci.filter) {
		case Renderer::TextureSamplerFilter::Linear:
			filter = GL_LINEAR;
			break;
		case Renderer::TextureSamplerFilter::Nearest:
			filter = GL_NEAREST;
			break;
		default:
			assert(0);
			break;
		}
		glCreateTextures(ci.type == Renderer::TextureType::Tex2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP, 1, &_textureID);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_S, wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_T, wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_WRAP_R, wrapMode);
		glTextureParameteri(_textureID, GL_TEXTURE_BASE_LEVEL, 0);
		glTextureParameteri(_textureID, GL_TEXTURE_MAX_LEVEL, ci.mipMaps ? 1000 : 0);
		glTextureParameteri(_textureID, GL_TEXTURE_MIN_FILTER, filter);
		glTextureParameteri(_textureID, GL_TEXTURE_MAG_FILTER, filter);
		glTextureStorage2D(_textureID, 1, internalformat, ci.width, ci.height);
		GLERR();
		int faces = ci.type == Renderer::TextureType::TexCube ? 6 : 1;
		_type = ci.type;
		if (ci.pixels) {
			if (Renderer::TextureType::TexCube == ci.type) {

				const uint8_t* data = ci.pixels;
				uint32_t size = (bytesperchannel*channels * ci.width * ci.height);
				for (unsigned i = 0; i < 6; ++i) {
					uint32_t offset = (uint32_t)(i * size);
					glTextureSubImage3D(_textureID, 0, 0, 0, i, ci.width, ci.height, 1, glfmt, gltype, &data[offset]);
					//data += ci.width * ci.height * bytesperchannel * channels;
					GLERR();
				}
			}
			else {
				glTextureSubImage2D(_textureID, 0, 0, 0, ci.width, ci.height, glfmt, gltype, ci.pixels);
				GLERR();
			}
		}
		_width = tex.width = ci.width;
		_height = tex.height = ci.height;
		_fmt = ci.format;
		tex.format = glfmt;
		tex.filter = filter;
		tex.wrapMode = wrapMode;
		tex.textureID = _textureID;
		tex.type = Renderer::TextureType::TexCube == ci.type ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
		//_size = vec2(ci.width, ci.height);
		_channels = channels;

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

	GLenum GLTexture::GetSamplerAddressMode(Renderer::TextureSamplerWrap addrMode)
	{
		GLenum mode = GL_CLAMP_TO_EDGE;
		switch (addrMode) {
		case Renderer::TextureSamplerWrap::Clamp:
			mode = GL_CLAMP_TO_EDGE;
			break;
		case Renderer::TextureSamplerWrap::Border:
			mode = GL_CLAMP_TO_BORDER;
			break;
		case Renderer::TextureSamplerWrap::Repeat:
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

	bool GLTexture::GetPixels(std::vector<uint8_t>& pixels, int face)const {
		int bytesperpixel = 0;
		GLenum format;
		GLenum dataType = GL_UNSIGNED_BYTE;
		bool isHalf = false;
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
		case Renderer::TextureFormat::FR16G16B16A16:
			isHalf = true;
			bytesperpixel = 4 * 2;
			format = GL_RGBA;
			dataType = GL_FLOAT;
			break;
		case Renderer::TextureFormat::FR32G32B32:
			bytesperpixel = 4 * 3;
			format = GL_RGB;
			dataType = GL_FLOAT;
			break;
		case Renderer::TextureFormat::FR32G32B32A32:
			bytesperpixel = 4 * 4;
			format = GL_RGBA;
			dataType = GL_FLOAT;
			break;
		default:
			assert(0);//format?
			break;
		}
		int facebytes = _width * _height * bytesperpixel;
		assert(facebytes <= pixels.size());
		if (pixels.size() < facebytes)
			return false;
		GLint currID;
		/*std::vector<uint8_t> data;
		if (isHalf)
			data.resize(facebytes);*/
		//void* p = isHalf ? data.data() : pixels.data();
		void* p = pixels.data();
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &currID);
		if (currID != _textureID)
			glBindTexture(GL_TEXTURE_2D, _textureID);
		if(_type == Renderer::TextureType::Tex2D)
			glGetTexImage(GL_TEXTURE_2D, 0, format, dataType, p);
		else {
			GLenum target;
			switch (face) {
			case 0:
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
				break;
			case 1:
				target = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
				break;
			case 2:
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
				break;
			case 3:
				target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
				break;
			case 4:
				target = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
				break;
			case 5:
				target = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
				break;
			default:
				assert(0);
				break;


			}
			glGetTexImage(target, 0, format, dataType, p);
		}
		if (currID != _textureID)
			glBindTexture(GL_TEXTURE_2D, currID);
		/*if (isHalf) {
			half_float::half* phalf = (half_float::half*)data.data();
			float* pf = (float*)pixels.data();
			for (uint32_t y = 0; y < _height; y++) {

				for (uint32_t x = 0; x < _width; x++) {
					uint32_t offset = x + y * _width;
					for (int j = 0; j < 4; j++) {

						half_float::half hv = *phalf++;
						float f = hv;
						*pf++ = f;
					}
				}
			}
		}*/
		
		return true;
	}
}