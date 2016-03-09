#include "AnimationManager.h"

namespace OvrMangaroll  {
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