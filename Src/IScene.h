#pragma once

#include "App.h"

using namespace OVR;

namespace OvrMangaroll {

	class IScene {
	public:
		virtual ~IScene() {}
		virtual void OneTimeInit(){}
		virtual void OneTimeShutdown(){}

		virtual void Frame(const VrFrame & vrFrame) = 0;
		virtual void DrawEyeView(const Matrix4f &eyeViewMatrix, const Matrix4f &eyeProjectionMatrix, const int eye) = 0;
		virtual void OnOpen() = 0;
		virtual void OnClose() = 0;
		virtual bool OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) = 0;
	protected:

	};
}