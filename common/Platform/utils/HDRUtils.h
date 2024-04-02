#pragma once
#include "../../Core/defines.h"
#include "../../Renderer/RenderDevice.h"
#include "../../Renderer/Texture.h"

namespace Utils {
	class HDRCubeMap {
		int32_t _width;
		int32_t _height;
		int32_t _componentsPerPixel;
		int32_t _bytesPerComp;
		std::vector<float> _pixels;
		Renderer::TextureFormat _format;
		HDRCubeMap(const char*path,bool isHDR);
		HDRCubeMap(Renderer::Texture* ptexture);
		HDRCubeMap(uint32_t faceSize);
		vec3 faceCoordsToXYZ(int i, int j, int faceID, int faceSize);
		vec4 getPixel(const float* pixels, int x, int y, int stride, int components);
		void setPixel(int x, int y,const vec4 &color);
	public:
		static HDRCubeMap* LoadCubeMap(const char* path);
		static HDRCubeMap* LoadCubeMap(Renderer::Texture* ptexture);
		static HDRCubeMap* CreateBlank(uint32_t faceSize);
		static HDRCubeMap* CreateFromHDR(const char* path);
		~HDRCubeMap();
		const uint8_t* getData()const;
		const int32_t getWidth()const;
		const int32_t getHeight()const;
		const int32_t getComponents()const;
		const Renderer::TextureFormat getFormat()const;
		void SaveToFile(const char* path);
		void SetFace(Renderer::Texture* ptexture, int face);
		Renderer::Texture* getTexture(Renderer::RenderDevice*pdevice);
		
	};
}