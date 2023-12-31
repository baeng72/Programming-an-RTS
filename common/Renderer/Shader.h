#pragma once
#include "RenderDevice.h"
#include "ShaderManager.h"
#include "Texture.h"
#include "Buffer.h"

namespace Renderer {
	
	class Shader {
	public:		
		static Shader * Create(RenderDevice* pdevice,void*shaderData);		
		virtual ~Shader() = default;		
		virtual void Bind(uint32_t* pdynoffsets = nullptr, uint32_t dynoffcount = 0) = 0;
		virtual void SetWireframe(bool wireframe)=0;
		//virtual void SetPushConstData(void*, uint32_t len)=0;		
		virtual bool SetUniformBuffer(uint32_t i, Buffer* pbuffer,bool dynamic=false) = 0;
		virtual bool SetUniformBuffer(const char* pname, Buffer* pbuffer,bool dynamic=false) = 0;
		virtual bool SetUniformData(uint32_t i,void*ptr,uint32_t len,bool dynamic=false) = 0;
		virtual bool SetUniformData(const char*pname, void* ptr, uint32_t len,bool dynamic=false) = 0;	
		virtual bool SetUniform(uint32_t i, vec2& v) = 0;
		virtual bool SetUniform(uint32_t i, vec2* p) = 0;
		virtual bool SetUniform(const char* pname, vec2& v) = 0;
		virtual bool SetUniform(const char* pname, vec2* p) = 0;
		virtual bool SetUniform(uint32_t i, vec3&v)=0;		
		virtual bool SetUniform(uint32_t i, vec3* p) = 0;
		virtual bool SetUniform(const char*pname, vec3&v) = 0;
		virtual bool SetUniform(const char*pname, vec3* p) = 0;
		virtual bool SetUniform(uint32_t i, vec4& v) = 0;
		virtual bool SetUniform(uint32_t i, vec4* p) = 0;
		virtual bool SetUniform(const char* pname, vec4& v) = 0;
		virtual bool SetUniform(const char* pname, vec4* p) = 0;
		virtual bool SetUniform(uint32_t i, mat4& v) = 0;
		virtual bool SetUniform(uint32_t i, mat4* p) = 0;
		virtual bool SetUniform(const char* pname, mat4& v) = 0;
		virtual bool SetUniform(const char* pname, mat4* p) = 0;
		virtual bool SetUniform(const char* pname, void* ptr, uint32_t size) = 0;
		virtual bool SetUniform(uint32_t i, void* ptr, uint32_t size) = 0;
		virtual uint32_t GetUniformId(const char* pname,bool dynamic=false) = 0;
		
		virtual bool SetTexture(uint32_t, Texture** pptexture, uint32_t count) = 0;		//set 1 or more textures into a slot/descriptor binding
		virtual bool SetTexture(const char*pname, Texture** pptexture, uint32_t count) = 0;		//set 1 or more textures into a slot/descriptor binding
		virtual bool SetTextures(Texture** pptextures, uint32_t count) = 0;						//set 1 or more textures into available texture slots/bindings
		virtual uint32_t GetTextureId(const char* pname) = 0;
		virtual bool SetStorageBuffer(uint32_t i, Buffer*pbuffer,bool dynamic=false) = 0;
		virtual bool SetStorageBuffer(const char* pname, Buffer*pbuffer,bool dynamic=false) = 0;
		virtual bool SetStorageData(uint32_t i, void* ptr, uint32_t len,bool dynamic=false)=0;
		virtual bool SetStorageData(const char* pname, void* ptr, uint32_t len,bool dynamic=false) = 0;		
		virtual uint32_t GetStorageId(const char* pname,bool dynamic=false) = 0;
		//virtual void Rebind(uint32_t* pdynoffsets = nullptr, uint32_t dynoffcount = 0) = 0;
		virtual void* GetNativeHandle() = 0;
		
	};
}
