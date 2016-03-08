#ifndef ANDROID
#define ANDROID
#endif

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

	GuideType AppState::Guide = GuideType::NONE;
	App *AppState::Instance = NULL;



	void *Web::DownloadFn(Thread *thread, void *p) {
		DownloadMeta *meta = (DownloadMeta *)p;
		LOG("DOWNLOADING %s", meta->url.ToCStr());
		JNIEnv *env;
		const ovrJava *java = AppState::Instance->GetJava();

		ovr_AttachCurrentThread(java->Vm, &env, NULL);
		jclass clazz = env->GetObjectClass(java->ActivityObject);
		jmethodID _LoadHttpUrl = env->GetStaticMethodID(clazz, "LoadHttpUrl", "(Ljava/lang/String;)[B");

		// Download stuff
		jstring jstr = env->NewStringUTF(meta->url.ToCStr());

		jbyteArray arr = (jbyteArray)env->CallStaticObjectMethod(clazz, _LoadHttpUrl, jstr);

		int count = env->GetArrayLength(arr);

		// Copy array
		void *buffer = env->GetPrimitiveArrayCritical(arr, 0);

		meta->callback(buffer, count, meta->target);

		env->ReleasePrimitiveArrayCritical(arr, buffer, 0);

		// Clean up
		env->DeleteLocalRef(jstr);
		env->DeleteLocalRef(arr);
		env->DeleteLocalRef(clazz);

		ovr_DetachCurrentThread(java->Vm);

		LOG("FINISHED DOWNLOADING %s", meta->url.ToCStr());
		delete meta;
		return NULL;
	}

	Thread *Web::Download(String url, DownloadCallback cb, void *obj) {
		DownloadMeta *meta = new DownloadMeta();
		meta->callback = cb;
		meta->target = obj;
		meta->url = url;

		Thread *thread = new Thread(
			Thread::CreateParams(DownloadFn, meta)
		);
		thread->Start();

		return thread;
	}

	AnimationManager::AnimationManager()
	: _Tasks()
	{}

	void AnimationManager::Progress(float delta) {
		for (int i = _Tasks.GetSizeI() - 1; i >= 0; i--) {
			AnimationTask *t = _Tasks[i];
			t->t = OVRMath_Min(1.0f, t->t + delta / t->duration);
			t->Progress();
			WARN("ANIMATE: %.2f", t->t);


			if (t->t >= 1.0f) {
				_Tasks.RemoveAt(i);
				delete t;
			}
		}
	}

	void AnimationManager::AddTask(AnimationTask *task) {
		_Tasks.PushBack(task);
	}
}