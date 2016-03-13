#ifndef ANDROID
#define ANDROID
#endif

#include "Web.h"
#include "Kernel\OVR_LogUtils.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "VrApi.h"
#include "Helpers.h"
#include <stdio.h> // for sscanf

namespace OvrMangaroll {

	ovrMessageQueue Web::S_Queue = ovrMessageQueue(100);
	Thread *Web::S_DownloadThread = new Thread(Thread::CreateParams(Web::S_ConsumerFn, NULL, 128 * 1024, -1, Thread::ThreadState::Running, Thread::BelowNormalPriority));

	void *Web::S_ConsumerFn(Thread *thread, void *p) {
		thread->SetThreadName("Web Worker Thread");
		while (!thread->GetExitFlag()) {
			S_Queue.SleepUntilMessage();
			const char *msg = S_Queue.GetNextMessage();
			if (msg != NULL) {
				QueueCb cb;
				void *obj;
				sscanf(msg, "call %p %p", &cb, &obj);

				cb(thread, obj);
			}
		}
		return NULL;
	}

	void *Web::DownloadFn(Thread *,void *p) {
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

	void Web::Download(String url, DownloadCallback cb, void *obj) {
		DownloadMeta *meta = new DownloadMeta();
		meta->callback = cb;
		meta->target = obj;
		meta->url = url;

		S_Queue.PostPrintf("call %p %p", DownloadFn, meta);
	}

}