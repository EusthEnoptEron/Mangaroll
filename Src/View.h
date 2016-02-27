/************************************************************************************

Filename    :   View.h
Content     :
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#if !defined( View_h )
#define View_h

#include "KeyState.h"
#include "Kernel/OVR_Math.h"
#include "Input.h"

using namespace OVR;


namespace OvrMangaroll {

	class View
	{
	protected:
		View(const char * name);

	public:
		const char *		name;

		enum eViewState {
			VIEWSTATE_CLOSED,
			VIEWSTATE_OPEN,
		};

		virtual 			~View();

		virtual void 		OneTimeInit(const char * launchIntent) = 0;
		virtual void		OneTimeShutdown() = 0;

		virtual void 		OnOpen() = 0;
		virtual void 		OnClose() = 0;

		virtual void		EnteredVrMode();  // By default do nothing.  Subclasses override.
		virtual void 		LeavingVrMode();  // By default do nothing.  Subclasses override.

		virtual bool 		OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) = 0;
		virtual Matrix4f 	Frame(const VrFrame & vrFrame) = 0;
		virtual Matrix4f	GetEyeViewMatrix(const int eye) const = 0;
		virtual Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const = 0;
		virtual Matrix4f 	DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) = 0;

		bool				IsOpen() const { return CurViewState == VIEWSTATE_OPEN; }
		bool				IsClosed() const { return CurViewState == VIEWSTATE_CLOSED; }

	protected:
		eViewState			CurViewState;		// current view state
		eViewState			NextViewState;		// state the view should go to on next frame
	};

}

#endif // Menu_h