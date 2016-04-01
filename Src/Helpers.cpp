#include "Helpers.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "VrApi.h"
#include "Kernel\OVR_Threads.h"
#include "App.h"
#include "Kernel\OVR_Math.h"

using namespace OVR;
namespace OvrMangaroll {
	double Time::Delta = 0;
	double Time::Elapsed = 0;

	const VrFrame *Frame::Current = NULL;

	Vector3f HMD::Direction = Vector3f(0, 0, -1.0f);
	//Vector3f HMD::Position = Vector3f(0, 0, 0);
	//Quatf HMD::Orientation = Quatf();

	float HMD::HorizontalAngle = 0;
	float HMD::VerticalAngle = 0;

	App *AppState::Instance = NULL;
	jclass AppState::FetcherClass = NULL;
	jclass AppState::ContainerFetcherClass = NULL;
	jclass AppState::MangaFetcherClass = NULL;
	jclass AppState::ActivityClass = NULL;
	Config *AppState::Conf = NULL;
	ThreadPool *AppState::Scheduler = NULL;

	Assets *Assets::_Instance = NULL;


	String MangaStringUtils::CropToLength(String base, int maxLength) {
		if (base.GetLengthI() < maxLength) {
			return base;
		}

		base = base.Left(maxLength - 3);
		return base + "...";
	}

	const char * const ParamString::PARAM_ID = "{id}";
	const char * const ParamString::PARAM_PAGE = "{page}";

	String ParamString::InsertParam(const char *source, const char *paramName, const char *value) {
		const ovrJava  *java = AppState::Instance->GetJava();
		JNIEnv *env = java->Env;
		JavaClass clazz = JavaClass(env, env->GetObjectClass(java->ActivityObject));

		jmethodID _InsertParam = env->GetStaticMethodID(clazz.GetJClass(), "InsertParam", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

		// Prepare params
		JavaString src = JavaString(env, source);
		JavaString param = JavaString(env, paramName);
		JavaString jvalue = JavaString(env, value);	
		JavaUTFChars result = JavaUTFChars(env, (jstring)env->CallStaticObjectMethod(clazz.GetJClass(), _InsertParam, src.GetJString(), param.GetJString(), jvalue.GetJString()));
		String resultString(result.ToStr());
		return resultString;
	}
}