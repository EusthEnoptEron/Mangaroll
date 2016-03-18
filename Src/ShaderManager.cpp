#include "ShaderManager.h"
#include "PackageFiles.h"
#include "Kernel\OVR_String_Utils.h"
#include "App.h"
using namespace OVR;
namespace OvrMangaroll {
	ShaderManager::ShaderManager() {
		
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

		GlProgram *program = _LoadedPrograms.Get(key);
		if (program == NULL) {
			// Add new program
			
			String vertexShader = LoadShader(vertPath);
			String fragShader = LoadShader(fragPath);
			GlProgram prog = BuildProgram(vertexShader.ToCStr(), fragShader.ToCStr());

			_LoadedPrograms.Add(key, prog);
			program = _LoadedPrograms.Get(key);
		}
		else {
			WARN("FOUND SHADER: %s", vertPath.ToCStr());
		}

		return program;
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