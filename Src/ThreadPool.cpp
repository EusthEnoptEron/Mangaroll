#include "ThreadPool.h"
#include "Helpers.h"

namespace OvrMangaroll {
	ThreadPool::ThreadPool(int concurrency) 
		: ThreadPool(concurrency, Thread::CreateParams(S_ConsumerFn, this)) {
		
	}
	ovrMessageQueue ThreadPool::S_Queue = ovrMessageQueue(200);

	ThreadPool::ThreadPool(int concurrency, Thread::CreateParams params) {
		for (int i = 0; i < concurrency; i++) {
			_Threads.PushBack(new Thread(params));
			_Threads.Back()->Start();
		}
	}

	void ThreadPool::Schedule(ThreadAction action, void*p) {
		S_Queue.PostPrintf("call %p %p", action, p);
	}

	void *ThreadPool::S_ConsumerFn(Thread *thread, void *p) {
		thread->SetThreadName("Worker Thread");		
		const ovrJava *java = AppState::Instance->GetJava();
		JNIEnv *env = java->Env;
		ovr_AttachCurrentThread(java->Vm, &env, NULL);

		JNIThread jniThread(thread, env);

		while (!thread->GetExitFlag()) {
			S_Queue.SleepUntilMessage();
			const char *msg = S_Queue.GetNextMessage();

			if (msg != NULL) {
				ThreadAction cb;
				void *obj;
				sscanf(msg, "call %p %p", &cb, &obj);

				cb(&jniThread, obj);
			}
		}

		ovr_DetachCurrentThread(java->Vm);
		return NULL;
	}

}