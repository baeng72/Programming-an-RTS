#pragma once

#include "RenderDevice.h"
#include "Texture.h"
namespace Renderer {
	
	class Material {
		
	public:
		enum class MaterialType { DiffuseColor, DiffuseTexture, DiffuseColorArray, DiffuseTextureArray, };
		static Material* Create(RenderDevice* pdevice, Material::MaterialType type);
		virtual ~Material()=default;
		virtual void SetAttributes(void* ptr, uint32_t size) = 0;		//pass a buffer of 1 or more elements (32-bit size) that holds triangle index
		virtual void SetData(void* ptr, uint32_t size)=0;				//pass in a buffer of some structure holding material info, each shader will determine what the type is
		virtual void SetTextures(Texture*ptr, uint32_t size) = 0;			//pass in an array (1 or more) textures, each shader will determine meaning.
		virtual void* GetMaterialData() = 0;
		virtual Material::MaterialType GetMaterialType() = 0;
	};
}
