#pragma once
#include "Kernel\OVR_String.h"
#include "VRMenuObject.h"
#include "MessageQueue.h"

using namespace OVR;
namespace OvrMangaroll {
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