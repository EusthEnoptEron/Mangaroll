#include "Mangaroll.h"

#include "RemotePage.h"
#include "jni.h"
#include "Android/JniUtils.h"

using namespace OVR;

namespace OvrMangaroll {
	RemotePage::RemotePage(String path, const ovrJava *java) : 
		Page(path), 
		_Java(java) 
	{
	}

	void *RemotePage::DownloadImage(Thread *thread, void *v) {
		RemotePage *page = (RemotePage *)v;

		JNIEnv *env;
		ovr_AttachCurrentThread(page->_Java->Vm, &env, NULL);

		page->_Clazz = env->GetObjectClass(page->_Java->ActivityObject);
		page->_LoadHttpUrl = env->GetStaticMethodID( page->_Clazz, "LoadHttpUrl", "(Ljava/lang/String;)[B");		


		// Download stuff
		jstring jstr = env->NewStringUTF( page->_Path.ToCStr() );

		jbyteArray arr = (jbyteArray) env->CallStaticObjectMethod(page->_Clazz, page->_LoadHttpUrl, jstr);
		
		int count = env->GetArrayLength(arr);

		// Copy array
		void *buffer = env->GetPrimitiveArrayCritical(arr, 0);
		page->ConsumeBuffer((unsigned char*)buffer, count);

		// Clean up
		env->ReleasePrimitiveArrayCritical(arr, buffer, 0);
		env->DeleteLocalRef(jstr);
		env->DeleteLocalRef(arr);

		ovr_DetachCurrentThread(page->_Java->Vm);

		return NULL;
	}

	Thread::ThreadFn RemotePage::GetWorker() {
		return &RemotePage::DownloadImage;
	}
}