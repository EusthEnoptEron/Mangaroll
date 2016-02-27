#pragma once

#include "GlProgram.h"
#include "Kernel\OVR_String.h"
#include "Kernel\OVR_StringHash.h"

using namespace OVR;
namespace OvrMangaroll {
	const char * const PAGE_SHADER_NAME = "assets/MangaPage.vert";

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