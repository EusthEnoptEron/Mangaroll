#pragma once

#include "VRMenuComponent.h"
#include "GuiSys.h"
#include "GazeCursor.h"

using namespace OVR;
class GazeUpdaterComponent : public VRMenuComponent {
public:

	GazeUpdaterComponent() : VRMenuComponent(VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED) | VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_FRAME_UPDATE) {

	}
private:

	eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		switch (event.EventType) {
		case VRMENU_EVENT_FOCUS_GAINED:
			guiSys.GetGazeCursor().ForceDistance(event.HitResult.t, eGazeCursorStateType::CURSOR_STATE_HILIGHT);
		default:
			return eMsgStatus::MSG_STATUS_ALIVE;

			break;
		}

	}

};