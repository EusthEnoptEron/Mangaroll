#pragma once

#include "Page.h"

namespace OvrMangaroll {
	class RemotePage : public Page {
		public:
			RemotePage(String path, const ovrJava *java);

		protected:
			Thread::ThreadFn virtual GetWorker();
			const ovrJava *_Java;
			jmethodID _LoadHttpUrl;
			jclass _Clazz;
			static void *DownloadImage(Thread *thread, void *v);
	};
}