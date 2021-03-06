#pragma once

#include "VRMenuComponent.h"
#include "GuiSys.h"
#include "GazeCursor.h"
#include "Helpers.h"
using namespace OVR;
namespace OvrMangaroll {
	class GazeUpdaterComponent : public VRMenuComponent {
	public:

		GazeUpdaterComponent() : VRMenuComponent(
			VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED) | VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_FRAME_UPDATE) {

		}
	private:

		eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event) {
			switch (event.EventType) {
			case VRMENU_EVENT_FOCUS_GAINED:
				self->SetHilighted(true);
			case VRMENU_EVENT_FRAME_UPDATE:
				//if (self->IsHilighted()) {
				if (event.HitResult.t < 10) {
					guiSys.GetGazeCursor().ForceDistance(event.HitResult.t - 0.1f, eGazeCursorStateType::CURSOR_STATE_HILIGHT);
				}
				//}
			case VRMENU_EVENT_FOCUS_LOST:
				self->SetHilighted(false);
			default:
				return eMsgStatus::MSG_STATUS_ALIVE;
				break;
			}
		}
	};

	class ClickableComponent : public VRMenuComponent {
	public:
		ClickableComponent()
			: ClickableComponent(VRMenuEventFlags_t())
		{
		}

		ClickableComponent(VRMenuEventFlags_t flags)
			: VRMenuComponent(VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED) | VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_TOUCH_DOWN | VRMENU_EVENT_TOUCH_UP | flags)
			, _IsFocused(false)
			, _IsClicked(false)
			, _Callback(NULL)
			, _CallbackTarget(NULL)
		{
		}

		void SetCallback(void(*cb)(void *p), void *p) {
			_Callback = cb;
			_CallbackTarget = p;
		}
	protected:
		virtual eMsgStatus _OnEvent(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event) {
			return MSG_STATUS_ALIVE;
		}

		virtual eMsgStatus _OnClick(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event) {
			if (_Callback != NULL) {
				_Callback(_CallbackTarget);
				return MSG_STATUS_CONSUMED;
			}
			return _OnEvent(guiSys, vrFrame, self, event);
		}
	private:

		eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event) {
			float timeTouchHasBeenDown = 0;
			float dist = 0;
			switch (event.EventType) {
			case VRMENU_EVENT_FOCUS_GAINED:
				_IsFocused = true;
				break;
			case VRMENU_EVENT_FOCUS_LOST:
				_IsFocused = false;
				_IsClicked = false;
				break;
			case VRMENU_EVENT_TOUCH_DOWN:
				_IsClicked = true;
				_ClickTime = Time::Elapsed;
				break;
			case VRMENU_EVENT_TOUCH_UP:
				timeTouchHasBeenDown = Time::Elapsed - _ClickTime;
				dist = event.FloatValue.LengthSq();

				if (dist < 20.0f && timeTouchHasBeenDown < 1.0f) {
					return _OnClick(guiSys, vrFrame, self, event);
				}
				break;
			default:
				break;
			}
			return _OnEvent(guiSys, vrFrame, self, event);
		}

		bool _IsFocused;
		bool _IsClicked;
		double _ClickTime;
		void(*_Callback)(void *);
		void *_CallbackTarget;
	};
}
