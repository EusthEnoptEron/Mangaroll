/************************************************************************************

Filename    :   ViewManager.cpp
Content     :
Created     :	6/17/2014
Authors     :   Jim Dosé

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the Cinema/ directory. An additional grant
of patent rights can be found in the PATENTS file in the same directory.

*************************************************************************************/

#include "ViewManager.h"
#include "App.h"

namespace OvrMangaroll {

	ViewManager::ViewManager() :
		LastCenterViewMatrix(),
		Views(),
		CurrentView(NULL),
		NextView(NULL),
		ClosedCurrent(false)

	{
		LastEyeViewMatrix[0].SetIdentity();
		LastEyeViewMatrix[1].SetIdentity();
		LastEyeProjectionMatrix[0].SetIdentity();
		LastEyeProjectionMatrix[1].SetIdentity();
		LastEyeViewProjectionMatrix[0].SetIdentity();
		LastEyeViewProjectionMatrix[1].SetIdentity();
	}

	void ViewManager::AddView(View * view)
	{
		LOG("AddView: %s", view->name);
		Views.PushBack(view);
	}

	void ViewManager::RemoveView(View * view)
	{
		for (UPInt i = 0; i < Views.GetSize(); i++)
		{
			if (Views[i] == view)
			{
				Views.RemoveAt(i);
				return;
			}
		}

		// view wasn't in the array
		assert(1);
		LOG("RemoveView: view not in array");
	}

	void ViewManager::OpenView(View & view)
	{
		LOG("OpenView: %s", view.name);
		NextView = &view;
		ClosedCurrent = false;
	}

	void ViewManager::CloseView()
	{
		if (CurrentView != NULL)
		{
			LOG("CloseView: %s", CurrentView->name);
			CurrentView->OnClose();
		}
	}

	void ViewManager::EnteredVrMode()
	{
		if (CurrentView != NULL)
		{
			LOG("EnteredVrMode: %s", CurrentView->name);
			CurrentView->EnteredVrMode();
		}
	}

	void ViewManager::LeavingVrMode()
	{
		if (CurrentView != NULL)
		{
			LOG("LeavingVrMode: %s", CurrentView->name);
			CurrentView->LeavingVrMode();
		}
	}

	bool ViewManager::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType)
	{
		if ((CurrentView != NULL) && !CurrentView->IsClosed())
		{
			return CurrentView->OnKeyEvent(keyCode, repeatCount, eventType);
		}
		else
		{
			return false;
		}
	}

	Matrix4f ViewManager::GetEyeViewMatrix(const int eye) const
	{
		if (CurrentView != NULL)
		{
			LastEyeViewMatrix[eye] = CurrentView->GetEyeViewMatrix(eye);
		}
		return LastEyeViewMatrix[eye];
	}

	Matrix4f ViewManager::GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const
	{
		if (CurrentView != NULL)
		{
			LastEyeProjectionMatrix[eye] = CurrentView->GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
		}
		return LastEyeProjectionMatrix[eye];
	}

	Matrix4f ViewManager::DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms)
	{
		if (CurrentView != NULL)
		{
			LastEyeViewMatrix[eye] = CurrentView->GetEyeViewMatrix(eye);
			LastEyeProjectionMatrix[eye] = CurrentView->GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
			LastEyeViewProjectionMatrix[eye] = CurrentView->DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
		}

		return LastEyeViewProjectionMatrix[eye];
	}

	Matrix4f ViewManager::Frame(const VrFrame & vrFrame)
	{
		if ((NextView != NULL) && (CurrentView != NULL) && !ClosedCurrent)
		{
			LOG("OnClose: %s", CurrentView->name);
			CurrentView->OnClose();
			ClosedCurrent = true;
		}

		if ((CurrentView == NULL) || (CurrentView->IsClosed()))
		{
			CurrentView = NextView;
			NextView = NULL;
			ClosedCurrent = false;

			if (CurrentView != NULL)
			{
				LOG("OnOpen: %s", CurrentView->name);
				CurrentView->OnOpen();
			}
		}

		if (CurrentView != NULL)
		{
			LastCenterViewMatrix = CurrentView->Frame(vrFrame);
		}

		return LastCenterViewMatrix;
	}

} // namespace OculusCinema
