#pragma once

#include "VrApi.h"
#include "App.h"
#include "UI/UIObject.h"
#include "UI/UIMenu.h"
#include "UI/UITexture.h"
#include "UI/UILabel.h"
#include "UI/UIImage.h"
#include "VRMenuComponent.h"
#include "GazeUpdateComponent.h"

using namespace OVR;

namespace OvrMangaroll {

	class UICheckbox;
	// Handles clicks
	class UICheckboxComponent : public ClickableComponent {
	public:
		UICheckboxComponent(UICheckbox *checkbox);
		virtual	~UICheckboxComponent() { }

	protected:
		virtual eMsgStatus _OnClick(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);
		virtual eMsgStatus _OnEvent(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);

	private:
		UICheckbox *_Owner;
	};


	class UICheckbox : public UIObject {
		friend class UICheckboxComponent;
	public:
		UICheckbox(OvrGuiSys &guiSys, float width, float height);
		~UICheckbox();

		void AddToMenu(UIMenu *menu, UIObject *parent = NULL);
		void SetOnValueChanged(void(*)(UICheckbox *, void *, bool), void *);

		void SetText(String text);
		void SetChecked(bool checked);
		bool IsChecked() {
			return _Checked;
		}

		void SetHover(bool hover) {
			this->SetHilighted(hover);
			UpdateImage();
		}
	private:
		void Init();
		void UpdateImage();

		void(*_Callback)(UICheckbox *, void *, bool);
		void *_CallbackObject;
		float _Width;
		float _Height;
		bool _Checked;


		UITexture _TickTexture;
		UITexture _DisabledTexture;
		UITexture _ActiveTexture;
		UIImage _Tick;
		UIImage _Background;
		UILabel _Text;
		UICheckboxComponent _Component;
	};

}