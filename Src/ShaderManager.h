#pragma once

#include "GlProgram.h"
#include "Kernel\OVR_String.h"
#include "Kernel\OVR_StringHash.h"
#include <map>

using namespace OVR;
namespace OvrMangaroll {
	const char * const PAGE_SHADER_NAME = "assets/shaders/MangaPage.vert";
	const char * const PAGE_TRANSPARENT_FRAG_NAME = "assets/shaders/TransparentMangaPage.frag";
	const char * const SHADER_VERT_FLOOR = "assets/shaders/floor_lol.vert";
	const char * const SHADER_VERT_SIMPLE = "assets/shaders/simple.vert";


	class ShaderManager {
	public:
		static ShaderManager *Instance(void);
		GlProgram *Get(String vertPath, String fragPath = "");
	private:
		ShaderManager();
		ShaderManager(const ShaderManager &);
		std::map<String, GlProgram*> _LoadedPrograms;
		static ShaderManager *_Instance;
		String LoadShader(String path);
	};
}