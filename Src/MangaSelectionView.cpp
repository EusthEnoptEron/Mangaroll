#include "MangaSelectionView.h"
#include "Mangaroll.h"
#include "Helpers.h"
#include "DefaultComponent.h"
#include "Kernel\OVR_MemBuffer.h"
#include "AsyncTexture.h"
#include "GazeUpdateComponent.h"

namespace OvrMangaroll {

	MangaSelectionView::MangaSelectionView(Mangaroll &app)
		: View("MangaSelectionView")
		, _Mangaroll(app)
		, _Menu(NULL)
		, _MainContainer(NULL)
		, _LocalSrcLabel(NULL)
		, _RemoteSrcLabel(NULL)
		, _Selector(NULL)
		, _FillTexture()
		, _LocalMangaProvider()
		, _NProvider("http://192.168.1.39:3000/browse/%d", "http://192.168.1.39:3000/show/%d")
		, _LocalCategoryComponent()
		, _RemoteCategoryComponent()
	{

	}

	MangaSelectionView::~MangaSelectionView() {

	}

	void OnSelectMangaLocal(Manga *manga, void *selectionView) {
		((MangaSelectionView *)selectionView)->OnSelectManga(manga);
	}


	void MangaSelectionView::OneTimeInit(const char *launchIntent) {
		OvrGuiSys &gui = _Mangaroll.GetGuiSys();
		GazeUpdaterComponent *gazeComponent = new GazeUpdaterComponent();


		_Menu = new UIMenu(gui);
		_Menu->Create("MangaSelectionMenu");
		_Menu->SetFlags(VRMenuFlags_t(VRMENU_FLAG_PLACE_ON_HORIZON) | VRMENU_FLAG_BACK_KEY_EXITS_APP);

		_MainContainer = new UIContainer(gui);
		_MainContainer->AddToMenu(_Menu);
		_MainContainer->SetLocalPosition(Vector3f(0, 0, -1));
		_MainContainer->AddComponent(gazeComponent);

		_SelectorContainer = new UIContainer(gui);
		_SelectorContainer->AddToMenu(_Menu);
		_SelectorContainer->SetLocalPosition(Vector3f(0, 0, -1));
		_SelectorContainer->AddComponent(gazeComponent);

		_FillTexture.LoadTextureFromApplicationPackage("assets/fill.png");


		_LocalSrcLabel = new UILabel(gui);
		_LocalSrcLabel->AddToMenu(_Menu, _MainContainer);
		_LocalSrcLabel->SetLocalPosition(Vector3f(-0.12f, 0, 0));
		_LocalSrcLabel->SetText("Local");
		_LocalSrcLabel->SetFontScale(0.5f);
		_LocalSrcLabel->CalculateTextDimensions();
		_LocalSrcLabel->SetImage(0, SURFACE_TEXTURE_DIFFUSE, _FillTexture, 100, 50);
		_LocalSrcLabel->SetColor(Vector4f(0, 0, 0, 1));
		_LocalSrcLabel->AddComponent(&_LocalCategoryComponent);
		//_LocalCategoryComponent.Selected = true;
		_LocalCategoryComponent.SetCallback(OnLocalCategory, this);

		_RemoteSrcLabel = new UILabel(gui);
		_RemoteSrcLabel->AddToMenu(_Menu, _MainContainer);
		_RemoteSrcLabel->SetLocalPosition(Vector3f(0.12f, 0, 0));
		_RemoteSrcLabel->SetText("Online");
		_RemoteSrcLabel->SetFontScale(0.5f);
		_RemoteSrcLabel->CalculateTextDimensions();
		_RemoteSrcLabel->SetImage(0, SURFACE_TEXTURE_DIFFUSE, _FillTexture, 100, 50);
		_RemoteSrcLabel->SetColor(Vector4f(0, 0, 0, 1));
		_RemoteSrcLabel->AddComponent(&_RemoteCategoryComponent);
		_RemoteCategoryComponent.SetCallback(OnRemoteCategory, this);



		_Selector = new MangaSelectorComponent(gui);
		_Selector->AddToMenu(_Menu, _SelectorContainer);
		_SelectorContainer->SetLocalPosition(Vector3f(0, -0.3f, -1));
		_Selector->SetOnSelectManga(OnSelectMangaLocal, this);
		_Selector->SetProvider(_NProvider);
		//_Selector->SetProvider(_LocalMangaProvider);
	}

	void MangaSelectionView::OnLocalCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;

		if (!self->_LocalCategoryComponent.Selected) {
			self->_LocalCategoryComponent.Selected = true;
			self->_RemoteCategoryComponent.Selected = false;

			self->_Selector->SetProvider(self->_LocalMangaProvider);
		}

	}
	void MangaSelectionView::OnRemoteCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;

		if (!self->_RemoteCategoryComponent.Selected) {
			self->_RemoteCategoryComponent.Selected = true;
			self->_LocalCategoryComponent.Selected = false;

			self->_Selector->SetProvider(self->_NProvider);
		}
	}

	void MangaSelectionView::OnSelectManga(Manga *manga) {
		_Mangaroll.ShowManga(manga);
	}

	void MangaSelectionView::OneTimeShutdown() {

	}

	void MangaSelectionView::OnOpen() {
		this->CurViewState = eViewState::VIEWSTATE_OPEN;
		_Menu->Open();
	}

	void MangaSelectionView::OnClose() {
		this->CurViewState = eViewState::VIEWSTATE_CLOSED;
		_Menu->Close();
	}

	bool MangaSelectionView::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) {
		return false;
	}
	Matrix4f MangaSelectionView::Frame(const VrFrame & vrFrame) {
		return _Mangaroll.Carousel.Frame(vrFrame);
	}
	Matrix4f MangaSelectionView::GetEyeViewMatrix(const int eye) const {
		return _Mangaroll.Carousel.GetEyeViewMatrix(eye);
	}
	Matrix4f MangaSelectionView::GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const {
		return _Mangaroll.Carousel.GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
	}
	Matrix4f MangaSelectionView::DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) {
		return _Mangaroll.Carousel.DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
	}

	//#######################################

	MangaPanel::MangaPanel(OvrGuiSys &guiSys) 
		: UIObject(guiSys)
	, _Cover(NULL) 
	, _CoverTexture(NULL)
	{

	}

	MangaPanel::~MangaPanel() {}

	void MangaPanel::AddToMenu(UIMenu *menu, UIObject *parent) {
		const Posef pose(Quatf(Vector3f(0.0f, 1.0f, 0.0f), 0.0f), Vector3f(0.0f, 0.0f, 0.0f));

		Vector3f defaultScale(1.0f);
		VRMenuFontParms fontParms(HORIZONTAL_CENTER, VERTICAL_CENTER, false, false, false, 1.0f);

		VRMenuObjectParms parms(VRMENU_CONTAINER, Array< VRMenuComponent* >(), VRMenuSurfaceParms(),
			"", pose, defaultScale, fontParms, menu->AllocId(),
			VRMenuObjectFlags_t(), VRMenuObjectInitFlags_t(VRMENUOBJECT_INIT_FORCE_POSITION));

		AddToMenuWithParms(menu, parent, parms);
		Init();
	}
	void MangaPanel::SetSelector(MangaSelectorComponent *selector) {
		_Component->Selector = selector;
	}

	Manga *MangaPanel::GetManga() {
		return _Manga;
	}
	void MangaPanel::SetManga(Manga *manga) {
		_Manga = manga;
		_Title->SetText(manga->Name);
		_Component->_Manga = _Manga;
		_Title->CalculateTextDimensions();

		_Cover->SetImage(0, SURFACE_TEXTURE_DIFFUSE, manga->GetCover()->Display(), 40, 60);
		WARN("%.2f, %f", _Cover->GetWorldPosition().z, _Title->GetDimensions().x);
		_Cover->AlignTo(RIGHT, _Title, LEFT);
		_Cover->SetLocalPosition(_Cover->GetLocalPosition() + Vector3f(0, 0, 0.05f));

	}

	void MangaPanel::Init(void) {
		UITexture *_BGTexture = new UITexture();
		_BGTexture->LoadTextureFromApplicationPackage("assets/fill.png");

		_Background = new UIImage(GuiSys);
		_Background->AddToMenu(Menu, this);
		_Background->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, *_BGTexture, 300, 50);
		_Background->SetMargin(UIRectf(60.0f));
		_Background->SetColor(Vector4f(0, 0, 0, 0.5f));
		_Background->SetPadding(UIRectf(10));

		_Title = new UILabel(GuiSys);
		_Title->AddToMenu(Menu, _Background);
		_Title->SetText("");
		_Title->SetTextOffset(Vector3f(0, 0, 0.04f));
		_Title->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 0.5));
		_Title->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_Title->SetMargin(UIRectf(50, 0, 0, 0));
		_Title->CalculateTextDimensions();
		
		_Cover = new UIImage(GuiSys);
		_Cover->AddToMenu(Menu, this);
		_Cover->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		
		_Component = new MangaPanelComponent(GuiSys);
		AddComponent(new OvrDefaultComponent());
		AddComponent(_Component);

	}

	//###########################################

	MangaPanelComponent::MangaPanelComponent(OvrGuiSys &guiSys)
		: VRMenuComponent(
		VRMenuEventFlags_t(VRMENU_EVENT_TOUCH_DOWN) | VRMENU_EVENT_FOCUS_GAINED | VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_TOUCH_UP)
		, Selector(NULL)
		, _Manga(NULL)
		, _Gui(guiSys)
		
	{
		
	}

	eMsgStatus MangaPanelComponent::OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {

		if (Selector != NULL && _Manga != NULL)  {
			switch (event.EventType) {
			case VRMENU_EVENT_TOUCH_DOWN:
				_Touched = _Focused;
				break;
			case VRMENU_EVENT_FOCUS_GAINED:
				_Focused = true;
				break;

			case VRMENU_EVENT_FOCUS_LOST:
				_Focused = false;
				_Touched = false;
				break;
			case VRMENU_EVENT_TOUCH_UP:
				if (_Touched) {
					Selector->SelectManga(_Manga);
					_Touched = false;
					return MSG_STATUS_CONSUMED;
				}
				break;
			default:
				break;
			}

		}

		return eMsgStatus::MSG_STATUS_ALIVE;
	}

	void MangaPanelComponent::AddToMenu(UIMenu *menu, UIObject *parent) {
		
	}
	//############################################


	MangaSelectorComponent::MangaSelectorComponent(OvrGuiSys &guiSys)
		: VRMenuComponent(
		VRMenuEventFlags_t(VRMENU_EVENT_FRAME_UPDATE) | VRMENU_EVENT_TOUCH_DOWN | VRMENU_EVENT_TOUCH_UP | VRMENU_EVENT_TOUCH_RELATIVE)
		, _Menu(NULL)
		, _Parent(NULL)
		, _Container(NULL)
		, _Gui(guiSys)
		, _Callback(NULL)
		, _CallbackObject(NULL)
		, _Panels()
		, _UsedPanels()
		, _Index(0)
		, _Speed(0)
		, _Gravity(0.1f)
		, _Provider(NULL)
	{

	}


	void MangaSelectorComponent::AddToMenu(UIMenu *menu, UIObject *parent) {
		WARN("I/DEBUG INITINITINIT");

		_Menu = menu;
		_Parent = parent;
		_Parent->AddComponent(this);
		_Container = new UIContainer(_Gui);
		_Container->AddToMenu(_Menu, _Parent);

		WARN("I/DEBUG NUM PANELS: %d", NUM_PANELS);
		// Create panels
		_Panels.Resize(NUM_PANELS);
		_PanelComponents.Resize(NUM_PANELS);
		for (int i = 0; i < NUM_PANELS; i++) {
			_Panels[i] = new MangaPanel(_Gui);
			_Panels[i]->AddToMenu(_Menu, _Container);
			_Panels[i]->SetSelector(this);
			_Panels[i]->SetVisible(false);
			if (i == 0) {
				_PanelHeight = _Panels[i]->GetMarginRect().GetHeight() * VRMenuObject::DEFAULT_TEXEL_SCALE;
			}
		}

		_PanelHeight = 0.2f;
	}

	// Populates the list of elements correctly
	void MangaSelectorComponent::RearrangePanels(void) {
		int minIndex = ceil(_Index - NUM_VISIBLE_PANELS);
		int maxIndex = floor(_Index + NUM_VISIBLE_PANELS);
		int startIndex = 1000;
		int endIndex = -1000;
		// Clean up
		for (int i = _UsedPanels.GetSizeI() - 1; i >= 0; i--) {
			if (_UsedPanels[i]->Index > maxIndex || _UsedPanels[i]->Index < minIndex) {
				_UsedPanels[i]->SetVisible(false);
				_UsedPanels[i]->GetManga()->GetCover()->Hide();
				_Panels.PushBack(_UsedPanels[i]);
				_UsedPanels.RemoveAt(i);
			}
			else {
				_UsedPanels[i]->GetManga()->GetCover()->Display();
				startIndex = Alg::Min(startIndex, _UsedPanels[i]->Index);
				endIndex = Alg::Max(endIndex, _UsedPanels[i]->Index);
			}
		}
		//LOG("I/DEBUG YEAH");
		int y = 0;
		for (int i = minIndex; i <= maxIndex; i++) {
			if (i >= 0 && i < _Provider->GetCurrentSize()) {
				//LOG("I/DEBUG %d - %d (%d) PANELS: %d", minIndex, maxIndex, i, _Panels.GetSizeI());

				if (i < startIndex || i > endIndex) {
					MangaPanel *panel = _Panels.Pop();
					panel->SetManga(_Provider->At(i));
					panel->SetVisible(true);

					panel->Index = i;
					_UsedPanels.PushBack(panel);
				}

				float diff = _UsedPanels[y]->Index - _Index;
				float distance = Alg::Abs(diff);

				_UsedPanels[y]->SetLocalPosition(Vector3f(0, 
					_PanelHeight * -(diff), -distance / 5));
				_UsedPanels[y]->SetColor(Vector4f(1, 1, 1, 1 - (distance / NUM_VISIBLE_PANELS)));

				y++;
			}
		}
	}


	void MangaSelectorComponent::SetProvider(MangaProvider &provider) {
		_Provider = &provider;
		_Index = 0;

		while (_UsedPanels.GetSizeI() > 0) {
			_Panels.PushBack(_UsedPanels.Pop());
		}
	}

	void MangaSelectorComponent::UpdatePositions(void) {
		// All right, now let's actually position those panels...
		//int baseIndex = ceil(_Index);
		//float progress = _Index - baseIndex;
		//_Container->SetLocalPosition(Vector3f(0, progress * _PanelHeight, 0));
	}

	void MangaSelectorComponent::SelectManga(Manga *manga) {
		if (_Callback != NULL) {
			_Callback(manga, _CallbackObject);
		}
	}

	void MangaSelectorComponent::Update(void) {
		// Update index

		if (!_IsTouching) {
			_Index += _Speed * Time::Delta;

			// Reduce speed
			_Speed = Alg::Lerp<float>(_Speed, 0, Time::Delta);

			int anchor = round(_Index);
			_Index = _Index + (anchor - _Index) * _Gravity * Time::Delta;// Alg::Lerp<float>(_Index, anchor, _Gravity * Time::Delta);

			_Index = Alg::Clamp<float>(_Index, 0, _Provider->GetCurrentSize() - 1);
		}
		if (_Provider != NULL) {
			if (!_Provider->IsLoading() && _Provider->HasMore()) {
				if (_Provider->GetCurrentSize() < _Index + 5) {
					_Provider->LoadMore();
				}
			}
		}
		RearrangePanels();
		UpdatePositions();
	}

	void MangaSelectorComponent::OnTouchDown(VrInput input) {
		_Speed = 0;
		_LastTouch = input.touch;
		_TouchStartTime = Time::Elapsed;
		_IsTouching = true;
	}
	void MangaSelectorComponent::OnTouchUp(VrInput input) {

		float elapsed = (Time::Elapsed - _TouchStartTime) * 500;
		_Speed = (input.touchRelative.y / -elapsed);
		_IsTouching = false;
	}
	void MangaSelectorComponent::OnTouchMove(VrInput input) {
		Vector2f diff = input.touch - _LastTouch;
		_Index -= diff.y / 500;
		_LastTouch = input.touch;
	}

	

	eMsgStatus MangaSelectorComponent::OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		if (_Provider == NULL) return;

		Vector2f diff();
		switch (event.EventType) {
		case VRMENU_EVENT_FRAME_UPDATE:
			Update();

			// Gotta do it ourselves...
			if (vrFrame.Input.buttonPressed & BUTTON_TOUCH) {
				OnTouchDown(vrFrame.Input);
			} 
			else if (vrFrame.Input.buttonState & BUTTON_TOUCH) {
				OnTouchMove(vrFrame.Input);
			}
			else if (vrFrame.Input.buttonReleased & BUTTON_TOUCH) {
				OnTouchUp(vrFrame.Input);
			}

			break;
		default:
			break;
		}

		return eMsgStatus::MSG_STATUS_ALIVE;
	}


	// ########### CATEGORY COMPONENT ###########

	CategoryComponent::CategoryComponent() 
		: ClickableComponent(VRMenuEventFlags_t(VRMENU_EVENT_FRAME_UPDATE) | VRMENU_EVENT_FOCUS_GAINED | VRMENU_EVENT_FOCUS_LOST)
		, Selected(false)
		, ColNormal(0,0,0,1)
		, ColFocused(.2, .2f, .4f, 1)
		, ColHighlight(.4f, .4f, .8f, 1)
		, _Focused(false)
	{
	}

	eMsgStatus CategoryComponent::_OnEvent(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {

		switch (event.EventType) {
		case VRMENU_EVENT_FOCUS_GAINED:
			_Focused = true;
			break;
		case VRMENU_EVENT_FOCUS_LOST:
			_Focused = false;
			break;
		case VRMENU_EVENT_FRAME_UPDATE:
			self->SetColor(
				Alg::Lerp(self->GetColor(), Selected ? ColHighlight : ( _Focused ? ColFocused : ColNormal ), Vector4f(vrFrame.DeltaSeconds * 10))
			);
			break;
		default:
			break;
		}

		return MSG_STATUS_ALIVE;
	}


}