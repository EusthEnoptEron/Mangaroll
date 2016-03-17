#pragma once

#include "GlProgram.h"
#include "Kernel\OVR_String.h"
#include "Kernel\OVR_StringHash.h"

using namespace OVR;
namespace OvrMangaroll {
	const char * const PAGE_SHADER_NAME = "assets/MangaPage.vert";
	const char * const PAGE_TRANSPARENT_FRAG_NAME = "assets/TransparentMangaPage.frag";

	class ShaderManager {
	public:
		static ShaderManager *Instance(void);
		GlProgram *Get(String vertPath, String fragPath = "");
	private:
		ShaderManager();
		ShaderManager(const ShaderManager &);
		StringHash<GlProgram> _LoadedPrograms;
		static ShaderManager *_Instance;
		String LoadShader(String path);
	};
}