#pragma once
#ifndef ANDROID
#define ANDROID
#endif

#include "jni.h"
#include "Kernel\OVR_Threads.h"
#include "MessageQueue.h"

using namespace OVR;
namespace OvrMangaroll {

	typedef Thread __THREAD;

	class JNIThread {
	public:
		JNIThread(Thread *t, JNIEnv *j)
			: thread(t)
			, Env(j)
		{}
		Thread *thread;
		JNIEnv *Env;
	};

	typedef void(*ThreadAction)(JNIThread *, void*);


	class ThreadPool {
	public:
		ThreadPool(int concurrency);
		ThreadPool(int concurrency, Thread::CreateParams);

		void Schedule(ThreadAction, void*);
	private:
		Array<Thread *> _Threads;
		
		static ovrMessageQueue S_Queue;
		static void *S_ConsumerFn(Thread *, void *);
	};
}