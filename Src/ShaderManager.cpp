#include "ShaderManager.h"
#include "PackageFiles.h"
#include "Kernel\OVR_String_Utils.h"
#include "App.h"
using namespace OVR;
namespace OvrMangaroll {
	ShaderManager::ShaderManager()
		: _LoadedPrograms()
	{
		
	}

	ShaderManager *ShaderManager::Instance(void) {
		if (_Instance == NULL) {
			_Instance = new ShaderManager();
		}
		return _Instance;
	}

	GlProgram *ShaderManager::Get(String vertPath, String fragPath) {
		if (fragPath == "") {
			// Default to same name
			fragPath = String(vertPath);
			fragPath.StripTrailing(".vert");
			fragPath += ".frag";
		}
		const String key = vertPath + fragPath;

		if (_LoadedPrograms.find(key) == _LoadedPrograms.end()) {
			// Add new program
			LOG("Load %s & %s", vertPath.ToCStr(), fragPath.ToCStr());
			String vertexShader = LoadShader(vertPath);
			String fragShader = LoadShader(fragPath);
			GlProgram prog = BuildProgram(vertexShader.ToCStr(), fragShader.ToCStr());
			LOG("Program that was built: %p", &prog);
			_LoadedPrograms[key] = new GlProgram(prog);

		}
		else {
			WARN("FOUND SHADER: %s", vertPath.ToCStr());
			//LOG("%s -> %d %d", vertPath.ToCStr(), program->program, program->fragmentShader);
		}
		
		return _LoadedPrograms[key];
	}

	String ShaderManager::LoadShader(String nameInZip) {
		void * 	buffer;	
		int		bufferLength;

		ovr_ReadFileFromApplicationPackage(nameInZip.ToCStr(), bufferLength, buffer);

		if (!buffer)
		{
			return "";
		}
	
		String resultString((char *)buffer, bufferLength);
		free(buffer);

		return resultString;
	}

	ShaderManager *ShaderManager::_Instance = NULL;
}