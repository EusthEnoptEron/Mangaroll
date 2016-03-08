#pragma once

#include "App.h"
#include "Kernel\OVR_Threads.h"
#include "VRMenuObject.h"
#include "Kernel\OVR_Array.h"
#include "Kernel\OVR_Alg.h"

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


	class AnimationTask {
	public: 
		AnimationTask(float duration, VRMenuObject *p)
			: t(0)
			, duration(duration)
			, p(p)
		{}

		virtual ~AnimationTask() {}

		float t;
		float duration;
		VRMenuObject *p;
		virtual void Progress() = 0;

	};

	template<class T>
	class TypedAnimationTask : public AnimationTask {
	public:
		TypedAnimationTask(float duration, VRMenuObject *p, T start, T end)
			: AnimationTask(duration, p)
			, start(start)
			, end(end)
		{
		}

		virtual ~TypedAnimationTask() {}
		T start;
		T end;
	};

	class AnimatePosition : public TypedAnimationTask<Vector3f> {
	public:
		AnimatePosition(VRMenuObject *obj, Vector3f to, float duration)
			: TypedAnimationTask(duration, obj, obj->GetLocalPosition(), to)
		{
		}

		virtual ~AnimatePosition() {
		}

		virtual void Progress() {
			p->SetLocalPosition(
				start.Lerp(end, t)
			);
		}
	};

	class AnimationManager {
	public:
		AnimationManager();

		// Task will be deleted upon finish
		void AddTask(AnimationTask *task);
		void Progress(float delta);
	private:
		Array<AnimationTask*> _Tasks;
	};
}