#pragma once

namespace Core {
	namespace ResourcePath {
		const char* GetResourcePath();
		void SetResourcePath(const char* path);
		void SetProjectPath(const char* projpath);
		const char* GetShaderPath(const char*proj,const char* name);
		const char* GetShaderPath(const char* name);
		const char* GetTexturePath(const char* proj, const char* name);
		const char* GetTexturePath(const char* name);
		const char* GetFontPath(const char* fontname);
		const char* GetMeshPath(const char* proj, const char* meshname);
		const char* GetMeshPath(const char* meshname);
		const char* GetSoundPath(const char* proj, const char* sndname);
		const char* GetSoundPath(const char* sndname);
		const char* GetMusicPath(const char* proj, const char* sndname);
		const char* GetMusicPath(const char* sndname);
		void GetMeshPath(const char* meshname, char* buffer, int len);
		const char* GetCursorPath(const char* proj, const char* cursorname);
		const char* GetCursorPath(const char* cursorname);
		const char* GetProjectResourcePath(const char* proj, const char* projresfile);
		const char* GetProjectResourcePath(const char* projresfile);
		bool ProjectResourcePathExists(const char* projresfile);
	}
}