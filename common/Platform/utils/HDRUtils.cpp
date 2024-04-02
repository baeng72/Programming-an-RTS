#include "HDRUtils.h"
#include "../../Core/Log.h"
#include <stb_image.h>
#include <stb_image_write.h>
namespace Utils {
	HDRCubeMap::HDRCubeMap(const char*path,bool isHDR)
		:_width(0),_height(0),_componentsPerPixel(0),_format(Renderer::TextureFormat::FR32G32B32A32),_bytesPerComp(4)
	{
		if (isHDR) {
			LOG_INFO("Loading HDR: {0}", path);
			int width, height, components;
			const float* img = stbi_loadf(path, &width, &height, &components, 4);
			assert(img);
			const int faceSize = width / 4;
			//conver equirectangular map to vertical cross (cube map)
			const int w = faceSize * 3;
			const int h = faceSize * 4;
			_width = w;
			_height = h;
			_componentsPerPixel = 4;
			_pixels.resize(w * h * _componentsPerPixel * _bytesPerComp);

			const ivec2 faceOffsets[] = {
				ivec2(faceSize, faceSize * 3),
				ivec2(0, faceSize),
				ivec2(faceSize, faceSize),
				ivec2(faceSize * 2,faceSize),
				ivec2(faceSize,0),
				ivec2(faceSize, faceSize * 2)
			};

			const int clampW = width - 1;
			const int clampH = height - 1;

			for (int face = 0; face < 6; face++) {
				for (int i = 0; i < faceSize; i++) {
					for (int j = 0; j < faceSize; j++) {
						const vec3 P = faceCoordsToXYZ(i, j, face, faceSize);
						const float R = std::hypot(P.x, P.y);
						const float theta = std::atan2(P.y, P.x);
						const float phi = std::atan2(P.z, R);
						//float point source coords
						const float Uf = float(2.f * faceSize * (theta + pi) / pi);
						const float Vf = float(2.f * faceSize * (pi * 0.5f - phi) / pi);
						//4-samples for bilinear interp
						const int U1 = clamp(int(floor(Uf)), 0, clampW);
						const int V1 = clamp(int(floor(Vf)), 0, clampH);
						const int U2 = clamp(U1 + 1, 0, clampW);
						const int V2 = clamp(V1 + 1, 0, clampH);
						//fractional part
						const float s = Uf - U1;
						const float t = Vf - V1;
						//fetch 4 samples
						const vec4 A = getPixel(img, U1, V1, width, _componentsPerPixel);
						const vec4 B = getPixel(img, U2, V1, width, _componentsPerPixel);
						const vec4 C = getPixel(img, U1, V2, width, _componentsPerPixel);
						const vec4 D = getPixel(img, U2, V2, width, _componentsPerPixel);
						//bilinear interpolation
						const vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) + C * (1 - s) * t + D * (s) * (t);
						setPixel(i + faceOffsets[face].x, j + faceOffsets[face].y, color);
					}
				}
			}

			stbi_image_free((void*)img);
		}
		else {
			LOG_INFO("Loading CubeMap: {0}", path);
			int width, height, components;
			
			const float* img = stbi_loadf(path, &width, &height, &components, 4);
			
			assert(img);
			_width = width;
			_height = height;
			_componentsPerPixel = 4;
			_pixels.resize(width * height * _componentsPerPixel * _bytesPerComp);
			memcpy(_pixels.data(), img, width * height * _componentsPerPixel * _bytesPerComp);
			stbi_image_free((void*)img);
		}
	}

	HDRCubeMap::HDRCubeMap(Renderer::Texture* ptexture) {
		LOG_INFO("loading cubemap from texture");
		int srcwidth, srcheight;
		ptexture->GetDimensions(srcwidth, srcheight);
		auto type = ptexture->GetType();
		assert(type == Renderer::TextureType::TexCube);
		auto format = ptexture->GetPixelFormat();
		int bytespercomp;
		int componentsperpixel;
		switch (format) {
		case Renderer::TextureFormat::FR32G32B32A32:
			bytespercomp = 4;
			componentsperpixel = 4;
			break;
		default:
			assert(0);
			break;
		}
		std::vector<uint8_t> facepixels(bytespercomp * componentsperpixel * srcwidth * srcheight);
		_width = 3*srcwidth;
		_height = 4*srcheight;
		_componentsPerPixel = componentsperpixel;
		_bytesPerComp = bytespercomp;
		_pixels.resize(_width * _height * _componentsPerPixel * _bytesPerComp);
		for (int i = 0; i < 6; i++) {
			ptexture->GetPixels(facepixels, i);
			int xstart = 0;
			int ystart = 0;
			switch (i) {
			case 0://positive x
				xstart = 0;
				ystart = srcheight;
				break;
			case 1://negative x
				xstart = 2 * srcwidth;
				ystart = srcheight;
				break;
			case 2://positive y
				xstart = srcwidth;
				ystart = 0;
				break;
			case 3://negative y
				xstart = srcwidth;
				ystart = 2 * srcheight;
				break;
			case 4://positive z
				xstart = srcwidth;
				ystart = srcheight;
				break;
			case 5://negative z
				xstart = srcwidth;
				ystart = 3 * srcheight;
				break;
				

				
			}
			float *p = (float*)facepixels.data();
			for (int y = 0; y < srcheight; y++) {
				for (int x = 0; x < srcwidth; x++) {
					const vec4 color = getPixel(p, x, y, srcwidth, componentsperpixel);
					setPixel(x + xstart, y + ystart, color);
				}
			}
		}
		
	}

	HDRCubeMap::HDRCubeMap(uint32_t faceSize) {
		const int w = faceSize * 3;
		const int h = faceSize * 4;
		_width = w;
		_height = h;
		_componentsPerPixel = 4;
		_bytesPerComp = 4;
		_pixels.resize(w * h * _componentsPerPixel * _bytesPerComp);
	}
	
	vec3 HDRCubeMap::faceCoordsToXYZ(int i, int j, int faceID, int faceSize) {
		const float A = 2.f * float(i) / faceSize;
		const float B = 2.f * float(j) / faceSize;
		switch (faceID) {
		case 0:
			return vec3(-1.f, A - 1.f, B - 1.f);
		case 1:
			return vec3(A - 1.f, -1.f, 1.f - B);
		case 2:
			return vec3(1.f, A - 1.f, 1.f - B);
		case 3:
			return vec3(1.f - A, 1.f, 1.f - B);
		case 4:
			return vec3(B - 1.f, A - 1.f, 1.f);
		case 5:
			return vec3(1.f - B, A - 1.f, -1.f);
		default:
			assert(0);
		}
		return vec3(0.f);
	}
	vec4 HDRCubeMap::getPixel(const float* pixels, int x, int y, int stride, int components)
	{
		const float* ptr = &pixels[components * (x + y * stride)];
		return glm::vec4(
			components > 0 ? ptr[0] : 0.f,
			components > 1 ? ptr[1] : 0.f,
			components > 2 ? ptr[2] : 0.f,
			components > 3 ? ptr[3] : 0.f
		);
	}
	void HDRCubeMap::setPixel(int x, int y, const vec4& color)
	{
		float* p = _pixels.data();
		p += _componentsPerPixel * (x + y * _width);
		if (_componentsPerPixel > 0)
			p[0] = color.x;
		if (_componentsPerPixel > 1)
			p[1] = color.y;
		if (_componentsPerPixel > 2)
			p[2] = color.z;
		if (_componentsPerPixel > 3)
			p[3] = color.w;		
	}
	HDRCubeMap* HDRCubeMap::CreateFromHDR(const char* path)
	{
		HDRCubeMap* pHDRCubeMap = new HDRCubeMap(path,true);
		
		return pHDRCubeMap;
	}
	HDRCubeMap* HDRCubeMap::LoadCubeMap(Renderer::Texture* ptexture){
		HDRCubeMap* pHDRCubeMap = new HDRCubeMap(ptexture);
		return pHDRCubeMap;
	}
	HDRCubeMap* HDRCubeMap::LoadCubeMap(const char* path) {
		HDRCubeMap* pHDRCubeMap = new HDRCubeMap(path, false);
		return pHDRCubeMap;
	}

	HDRCubeMap* HDRCubeMap::CreateBlank(uint32_t faceSize) {
		HDRCubeMap* pHDRCubeMap = new HDRCubeMap(faceSize);
		return pHDRCubeMap;
	}

	HDRCubeMap::~HDRCubeMap()
	{
	}
	const uint8_t* HDRCubeMap::getData() const
	{
		return nullptr;
	}
	const int32_t HDRCubeMap::getWidth() const
	{
		return _width;
	}
	const int32_t HDRCubeMap::getHeight() const
	{
		return _height;
	}
	const int32_t HDRCubeMap::getComponents() const
	{
		return _componentsPerPixel;
	}
	const Renderer::TextureFormat HDRCubeMap::getFormat() const
	{
		return _format;
	}

	void HDRCubeMap::SaveToFile(const char* path) {
		stbi_write_hdr(path, _width, _height, _componentsPerPixel, (const float*)_pixels.data());
	}

	void HDRCubeMap::SetFace(Renderer::Texture* ptexture, int face)
	{
		LOG_INFO("loading cubemap from texture");
		int srcwidth, srcheight;
		ptexture->GetDimensions(srcwidth, srcheight);
		/*auto type = ptexture->GetType();
		assert(type == Renderer::TextureType::TexCube);*/
		auto format = ptexture->GetPixelFormat();
		int bytespercomp;
		int componentsperpixel;
		switch (format) {
		case Renderer::TextureFormat::FR32G32B32A32:
			bytespercomp = 4;
			componentsperpixel = 4;
			break;
		default:
			assert(0);
			break;
		}
		std::vector<uint8_t> facepixels(bytespercomp * componentsperpixel * srcwidth * srcheight);
		ptexture->GetPixels(facepixels, 0);
		const int pixelSize = componentsperpixel * bytespercomp;
		uint8_t* pdst = (uint8_t*)_pixels.data();
		uint8_t* psrc = facepixels.data();
		for (int j = 0; j < srcheight; ++j) {
			for (int i = 0; i < srcwidth; ++i) {
				int x = 0;
				int y = 0;
				switch (face) {
				case 0:
					x = i;
					y = srcheight + j;
					break;
					// negative x
				case 1:
					x = 2 * srcwidth + i;
					y = srcheight + j;
					break;
				case 2: //positive y
					x = 2 * srcwidth - (i + 1);
					y = srcheight - (j + 1);
					break;
				case 3://negative y
					x = 2 * srcwidth - (i + 1);
					y = 3 * srcheight - (j + 1);
					break;
				case 4://positive z
					x = 2 * srcwidth - (i + 1);
					y = _height - (j + 1);
					break;
				case 5://negative z
					x = srcwidth + i;
					y = srcheight + j;
					
					break;



				}
				
				
				memcpy(pdst + (y * _width + x) * pixelSize,psrc, pixelSize);

				psrc += pixelSize;
				/*for (int y = 0; y < srcheight; y++) {
					if (y == 32) {
						int z = 0;
					}
					for (int x = 0; x < srcwidth; x++) {
						const vec4 color = getPixel(p, x, y, srcwidth, componentsperpixel);
						setPixel(x + xstart, y + ystart, color);
					}
				}*/
			}
		}
	}

	

	Renderer::Texture* HDRCubeMap::getTexture(Renderer::RenderDevice* pdevice) {
		//convert vertical cross to 6 faces 
		const int faceWidth = _width / 3;
		const int faceHeight = _height / 4;
		const uint8_t* src = (uint8_t*)_pixels.data();
		std::vector<uint8_t> dst(6 * faceWidth * faceHeight * _componentsPerPixel * _bytesPerComp);
		uint8_t* pdst = dst.data();
		/*
			------
			| +Y |
	 ----------------
	 | -X | -Z | +X |
	 ----------------
			| -Y |
			------
			| +Z |
			------
	*/
		const int pixelSize = _componentsPerPixel * _bytesPerComp;

		for (int face = 0; face < 6; ++face) {
			for (int j = 0; j < faceHeight; ++j) {
				for (int i = 0; i < faceWidth; ++i) {
					int x = 0;
					int y = 0;

					switch (face) {
						// positive x
					case 0:
						x = i;
						y = faceHeight + j;
						break;
						// negative x
					case 1:
						x = 2 * faceWidth + i;
						y = faceHeight + j;
						break;
					case 2: //positive y
						x = 2 * faceWidth - (i + 1);
						y = faceHeight - (j + 1);
						break;
					case 3://negative y
						x = 2 * faceWidth - (i + 1);
						y = 3 * faceHeight - (j + 1);
						break;
					case 4://positive z
						x = 2 * faceWidth - (i + 1);
						y = _height - (j + 1);
						break;
					case 5://negative z
						x = faceWidth + i;
						y = faceHeight + j;
						break;
					default:
						assert(0);
						break;

					}
					
					memcpy(pdst, src + (y * _width + x) * pixelSize, pixelSize);
					
					pdst += pixelSize;
				}
			}
		}
		
		Renderer::TextureCreateInfo ci;
		ci.width = faceWidth;
		ci.height = faceHeight;
		ci.type = Renderer::TextureType::TexCube;
		ci.wrap = Renderer::TextureSamplerWrap::Clamp;
		ci.mipMaps = 1;
		ci.format = Renderer::TextureFormat::FR32G32B32A32;
		ci.pixels = dst.data();
		ci.filter = Renderer::TextureSamplerFilter::Linear;
		return Renderer::Texture::Create(pdevice, ci);
	}
}

