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

	//class ParamString {
	//public:
	//	static String InsertParam(char *source, char *paramName, char *value) {
	//		const ovrJava  *java = AppState::Instance->GetJava();
	//		JNIEnv *env = java->Env;
	//		jclass clazz = env->GetObjectClass(java->ActivityObject);
	//		jmethodID _InsertParam = env->GetStaticMethodID(clazz, "InsertParam", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

	//		// Prepare params
	//		jstring src = env->NewStringUTF(source);
	//		jstring param = env->NewStringUTF(paramName);
	//		jstring value = env->NewStringUTF(value);

	//		jstring result = (jstring)env->CallStaticObjectMethod(clazz, _InsertParam, src, param, value);
	//		const char *resultString = env->GetStringUTFChars(result, JNI_TRUE);

	//		// Clean up
	//		

	//		return resultString;
	//	}

	//	static void InsertParam(String source, String paramName, int value) {
	//		
	//	}
	//};
}