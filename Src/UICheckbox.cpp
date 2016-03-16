#include "UICheckbox.h"


namespace OvrMangaroll {
	UICheckbox::UICheckbox(OvrGuiSys &guiSys, float width, float height)
		: UIObject(guiSys)
		, _Callback(NULL)
		, _CallbackObject(NULL)
		, _Width(width)
		, _Height(height)
		, _Checked(false)
		, _TickTexture()
		, _DisabledTexture()
		, _ActiveTexture()
		, _Tick(guiSys)
		, _Background(guiSys)
		, _Text(guiSys)
		, _Component(this)
	{
	}

	UICheckbox::~UICheckbox() {}

	void UICheckbox::AddToMenu(UIMenu *menu, UIObject *parent) {
		const Posef pose(Quatf(Vector3f(0.0f, 1.0f, 0.0f), 0.0f), Vector3f(0.0f, 0.0f, 0.0f));

		Vector3f defaultScale(1.0f);
		VRMenuFontParms fontParms(HORIZONTAL_CENTER, VERTICAL_CENTER, false, false, false, 1.0f);

		VRMenuObjectParms parms(VRMENU_BUTTON, Array< VRMenuComponent* >(), VRMenuSurfaceParms(),
			"", pose, defaultScale, fontParms, menu->AllocId(),
			VRMenuObjectFlags_t(), VRMenuObjectInitFlags_t(VRMENUOBJECT_INIT_FORCE_POSITION));

		AddToMenuWithParms(menu, parent, parms);

		Init();
	}

	void UICheckbox::Init() {
		AddComponent(&_Component);

		_Tick.AddToMenu(Menu, this);
		_Text.AddToMenu(Menu, this);
		_Background.AddToMenu(Menu, this);

		// Load images
		_DisabledTexture.LoadTextureFromApplicationPackage("assets/checkbox_base.png");
		_ActiveTexture.LoadTextureFromApplicationPackage("assets/checkbox_selected.png");
		_TickTexture.LoadTextureFromApplicationPackage("assets/tick.png");

		float offset = (-_Width / 2 + _Height / 2) * VRMenuObject::DEFAULT_TEXEL_SCALE;
		
		_Tick.SetImage(0, SURFACE_TEXTURE_DIFFUSE, _TickTexture, _Height, _Height);
		_Background.SetLocalPosition(Vector3f(offset, 0, 0));
		_Tick.SetLocalPosition(Vector3f(offset, 0, 0.01f));
		// Init text
		_Text.SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 1));
		_Text.SetLocalPosition(Vector3f(0, 0, 0.01f));
		_Text.SetFontScale(0.25f);
		_Text.SetText("");
		_Text.SetAlign(HorizontalJustification::HORIZONTAL_LEFT, VerticalJustification::VERTICAL_CENTER);
		UpdateImage();

		this->SetDimensions(Vector2f(_Width, _Height) * VRMenuObject::DEFAULT_TEXEL_SCALE);

	}

	void UICheckbox::SetOnValueChanged(void(*cb)(UICheckbox *, bool, void *), void *obj) {
		_CallbackObject = obj;
		_Callback = cb;
	}

	void UICheckbox::SetText(String text) {
		_Text.SetTextWordWrapped(text.ToCStr(), GuiSys.GetDefaultFont(), (_Width - _Height) * VRMenuObject::DEFAULT_TEXEL_SCALE);
		_Text.CalculateTextDimensions();
		Vector2f dims = _Text.GetDimensions();

		float x = (_Tick.GetLocalPosition().x / VRMenuObject::DEFAULT_TEXEL_SCALE)
			+ (_Height / 2 + 10) + dims.x / 2;

		_Text.SetLocalPosition(Vector3f(x * VRMenuObject::DEFAULT_TEXEL_SCALE, 0, 0.01f));
	}

	void UICheckbox::SetChecked(bool checked) {
		_Checked = checked;
		UpdateImage();

		if (_Callback != NULL) {
			_Callback(this, IsChecked(), _CallbackObject);
		}
	}

	void UICheckbox::UpdateImage() {

		if (IsHilighted()) {
			_Background.SetImage(0, SURFACE_TEXTURE_DIFFUSE, _ActiveTexture, _Height, _Height);
		}
		else {
			_Background.SetImage(0, SURFACE_TEXTURE_DIFFUSE, _DisabledTexture, _Height, _Height);
		}

		_Tick.SetVisible(_Checked);
	}



	// ############# COMPONENT ##############
	UICheckboxComponent::UICheckboxComponent(UICheckbox *checkbox)
		: ClickableComponent(VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED)
		   | VRMENU_EVENT_FOCUS_LOST)
		   , _Owner(checkbox)
	{

	}

	eMsgStatus UICheckboxComponent::_OnClick(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		_Owner->SetChecked(!_Owner->IsChecked());
		return MSG_STATUS_CONSUMED;
	}

	eMsgStatus UICheckboxComponent::_OnEvent(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		if (event.EventType == VRMENU_EVENT_FOCUS_GAINED) {
			_Owner->SetHover(true);
		}
		else if(event.EventType == VRMENU_EVENT_FOCUS_LOST) {
			_Owner->SetHover(false);

		}
		return MSG_STATUS_ALIVE;
	}

}