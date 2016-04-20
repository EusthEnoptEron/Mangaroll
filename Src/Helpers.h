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
#include "ThreadPool.h"
#include "TranslationManager.h"

using namespace OVR;
namespace OvrMangaroll {
	class Mangaroll;

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
		static Vector3f NormalizedDirection;
		static float HorizontalAngle;
		static float VerticalAngle;
	/*	static Quatf Orientation;
		static Vector3f Position;*/
	};

	enum GuideType {
		NONE, ENLARGE, FOLLOW
	};
	
	class Config;
	class AppState {
	public:
		static ThreadPool *Scheduler;
		static Config *Conf;
		static TranslationManager *Strings;
		static App *Instance;
		static Mangaroll *Reader;
		static jclass FetcherClass;
		static jclass ContainerFetcherClass;
		static jclass MangaFetcherClass;
		static jclass ActivityClass;
	};

	class Assets {
	public: 
		UITexture Preloader;
		UITexture Panel;
		UITexture Fill;
		UITexture ProgressBG;

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
			ProgressBG.LoadTextureFromApplicationPackage("assets/progress_bg.png");
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


	//==============================================================
	// JavaObject
	//
	// Releases a java object global reference on destruction.
	//==============================================================
	class JavaGlobalObject
	{
	public:
		JavaGlobalObject(JNIEnv * jni_, jobject const object_) :
			Jni(jni_),
			Object(object_)
		{
			OVR_ASSERT(Jni != NULL);
			Object = jni_->NewGlobalRef(object_);
		}
		~JavaGlobalObject()
		{
#if defined( OVR_OS_ANDROID )
			if (Jni->ExceptionOccurred())
			{
				LOG("JNI exception before DeleteLocalRef!");
				Jni->ExceptionClear();
			}
			OVR_ASSERT(Jni != NULL && Object != NULL);
			Jni->DeleteGlobalRef(Object);
			if (Jni->ExceptionOccurred())
			{
				LOG("JNI exception occured calling DeleteLocalRef!");
				Jni->ExceptionClear();
			}
#endif
			Jni = NULL;
			Object = NULL;
		}

		jobject			GetJObject() const { return Object; }

		JNIEnv *		GetJNI() const { return Jni; }

	protected:
		void			SetJObject(jobject const & obj) { Object = obj; }

	private:
		JNIEnv *		Jni;
		jobject			Object;
	};


	//==============================================================
	// JavaClass
	//
	// Releases a java class local reference on destruction.
	//==============================================================
	class JavaGlobalClass : public JavaGlobalObject
	{
	public:
		JavaGlobalClass(JNIEnv * jni_, jobject const class_) :
			JavaGlobalObject(jni_, class_)
		{
		}

		jclass			GetJClass() const { return static_cast< jclass >(GetJObject()); }
	};


	class Color {
	public:
		static Vector3f HSL2RGB(const Vector3f &hsl);
		static Vector3f RGB2HSL(const Vector3f &rgb);
	};
}