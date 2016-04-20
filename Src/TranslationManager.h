#pragma once

#include "Kernel\OVR_String.h"

using namespace OVR;
namespace OvrMangaroll {

	class Mangaroll;

	class TranslationManager {
	public:
		TranslationManager(Mangaroll *mangaroll);
	public: 
		String AppName;
		String ActionSettings;
		String NoLocalFiles;
		String NoServices;
		String NoResults;
		String LabelLocal;
		String LabelOnline;
		String PageNo;
		String PrefContrast;
		String PrefBrightness;
		String PrefSharpen;
		String PrefTransparent;
		String PrefReadDir;
		String PrefAutoProgress;
		String PrefHeadMotions;
	};
}