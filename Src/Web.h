#pragma once
#include "Kernel\OVR_String.h"
#include "Kernel\OVR_Threads.h"
#include "MessageQueue.h"
#include "AsyncTexture.h"

using namespace OVR;
namespace OvrMangaroll {
	typedef void(*DownloadCallback)(void *buffer, int length, void *obj);
	struct DownloadMeta {
		String url;
		void *target;
		DownloadCallback callback;
	};

	class Web {
	public:
		static void *DownloadFn(Thread *,void *p);
		static void Download(String url, DownloadCallback cb, void *obj);

	private:
		static ovrMessageQueue S_Queue;
		static Thread *S_DownloadThread;
		static void *S_ConsumerFn(Thread *, void *);
	};

}