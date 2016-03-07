#pragma once

#include "App.h"
#include "Kernel\OVR_Threads.h"

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

	typedef void(*DownloadCallback)(void *buffer, int length, void *obj);
	struct DownloadMeta {
		String url;
		void *target;
		DownloadCallback callback;
	};

	class Web {
	public:
		static void *DownloadFn(Thread *thread, void *p);
		static Thread *Download(String url, DownloadCallback cb, void *obj);
	};


}