/************************************************************************************

Filename    :   ViewManager.h
Content     :
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "Kernel/OVR_Array.h"
#include "Kernel/OVR_Math.h"
#include "Input.h"
#include "View.h"

#if !defined( ViewManager_h )
#define ViewManager_h

namespace OvrMangaroll {

	class ViewManager
	{
	public:
		ViewManager();

		View *				GetCurrentView() const { return CurrentView; };

		void 				AddView(View * view);
		void 				RemoveView(View * view);

		void 				OpenView(View & view);
		void 				CloseView();

		void				EnteredVrMode();
		void 				LeavingVrMode();

		bool 				OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType);
		Matrix4f 			Frame(const VrFrame & vrFrame);
		virtual Matrix4f	GetEyeViewMatrix(const int eye) const;
		virtual Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const;
		Matrix4f 			DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms);

	private:
		mutable Matrix4f	LastCenterViewMatrix;
		mutable Matrix4f	LastEyeViewMatrix[2];
		mutable Matrix4f	LastEyeProjectionMatrix[2];
		mutable Matrix4f	LastEyeViewProjectionMatrix[2];

		Array<View *> 		Views;

		View *				CurrentView;
		View *				NextView;

		bool				ClosedCurrent;
	};

}

#endif // ViewManager_h
