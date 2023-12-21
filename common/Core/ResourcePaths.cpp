#include "ResourcePaths.h"
#include <cstring>
#include <cstdio>
#include "Api.h"
namespace Core {
	namespace ResourcePath {
		char resourcePath[256]="../../../../Resources";
		char projPath[128];
		char filePath[512];
		const char* GetResourcePath()
		{
			return resourcePath;
		}
		void SetResourcePath(const char* path)
		{
			strcpy_s(resourcePath, path);
		}
		void SetProjectPath(const char* pname) {
			strcpy_s(projPath, pname);
		}
		const char* GetShaderPath(const char*proj,const char* name)
		{
			sprintf_s(filePath,"%s/%s/shaders/%s/%s", resourcePath, proj,GetAPI()==API::Vulkan ? "Vulkan" : "GL", name);
			return filePath;
		}
		const char* GetShaderPath(const char* name)
		{
			return GetShaderPath(projPath, name);
		}
		const char* GetTexturePath(const char* proj, const char* name) {
			sprintf_s(filePath, "%s/%s/textures/%s", resourcePath, proj, name);
			return filePath;
		}
		const char* GetTexturePath(const char* name) {
			return GetTexturePath(projPath, name);
		}
		const char* GetFontPath(const char* fontname)
		{
			sprintf_s(filePath, "%s/Fonts/%s", resourcePath, fontname);
			return filePath;
		}
		const char* GetCursorPath(const char*proj,const char* cursorname) {
			sprintf_s(filePath, "%s/%s/cursors/%s", resourcePath,proj, cursorname);
			return filePath;
		}
		const char* GetCursorPath(const char* cursorname) {
			return GetCursorPath(projPath, cursorname);
		}

		const char* GetProjectResourcePath(const char* proj, const char* projresfile)
		{
			sprintf_s(filePath, "%s/%s/%s", resourcePath, proj, projresfile);
			return filePath;
		}

		const char* GetProjectResourcePath(const char* projresfile) {
			return GetProjectResourcePath(projPath, projresfile);
		}
		
		const char* GetMeshPath(const char* proj, const char* meshname) {
			sprintf_s(filePath, "%s/%s/meshes/%s", resourcePath,proj, meshname);
			return filePath;
		}
		const char* GetMeshPath(const char* meshname) {
			return GetMeshPath(projPath, meshname);
		}
	}
}