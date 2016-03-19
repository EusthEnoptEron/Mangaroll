#pragma once
#ifndef ANDROID
#define ANDROID
#endif

#include "App.h"
#include "Kernel\OVR_Threads.h"
#include "VRMenuObject.h"
#include "Kernel\OVR_Array.h"
#include "Kernel\OVR_Alg.h"
#include "Kernel\OVR_String_Utils.h"
#include "PathUtils.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "UI\UITexture.h"
#include "UI\UIObject.h"

using namespace OVR;
namespace OvrMangaroll {

	// Holder for time information
	class Time {
	public:
		// Time in seconds that has passed since the last update
		static double Delta;
		// Time in seconds that has passed from the start of the application
		// until the last update
		static double Elapsed;
	};


	// Holder for input information
	class Frame {
	public:
		static const VrFrame *Current;
	};

	class HMD {
	public:
		static Vector3f Direction;
		static float HorizontalAngle;
		static float VerticalAngle;
	/*	static Quatf Orientation;
		static Vector3f Position;*/
	};

	enum GuideType {
		NONE, ENLARGE, FOLLOW
	};

	class AppState {
	public:
		static GuideType Guide;
		static bool Transparent;
		static bool LeftToRight;
		static bool AutoRotate;
		static float Contrast;
		static float Brightness;
		static float Zoom;
		static App *Instance;
	};

	class Assets {
	public: 
		UITexture Preloader;
		UITexture Panel;
		UITexture Fill;
		
		static Assets &Instance() {
			if (_Instance == NULL) {
				_Instance = new Assets();
			}
			return *_Instance;
		}
	private:
		Assets()
		: Preloader()
		, Panel()
		{
			Preloader.LoadTextureFromApplicationPackage("assets/preloader.png");
			Panel.LoadTextureFromApplicationPackage("assets/container.png");
			Fill.LoadTextureFromApplicationPackage("assets/fill.png");
		}
		static Assets *_Instance;
	};

	class MangaStringUtils {
	public:
		static String CropToLength(String, int maxLength);
	};

	class ParamString {
	public:
		static const char * const PARAM_PAGE;
		static const char * const PARAM_ID;

		static String InsertParam(const char *source, const char *paramName, const char *value);

		static String InsertParam(const char *source, const char *paramName, int value) {
			return InsertParam(source, paramName, String::Format("%d", value).ToCStr());
		}
	};
}