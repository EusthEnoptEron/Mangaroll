#include "MangaSelectionView.h"
#include "Mangaroll.h"
#include "Helpers.h"
#include "DefaultComponent.h"
#include "Kernel\OVR_MemBuffer.h"
#include "AsyncTexture.h"
#include "GazeUpdateComponent.h"
#include "AnimationManager.h"
#include "VRMenuMgr.h"
#include "Kernel\OVR_JSON.h"

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
		//, _NProvider("http://192.168.1.39:3000/nh/browse/%1$d", "http://192.168.1.39:3000/nh/show/%1$d")
		//, _NProvider("http://192.168.1.39:3000/mr/browse/%1$d?id=%2$s", "http://192.168.1.39:3000/mr/show/%1$d")
		//, _NProvider("http://192.168.1.39:3000/ya/browse/%1$d", "http://192.168.1.39:3000/ya/show/%1$d")
		, _NProvider()
		, _LocalCategoryComponent()
		, _RemoteCategoryComponent()
		, _Fader(0)
		, _CloseRequested(false)
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
		_Menu->SetFlags(VRMenuFlags_t(VRMENU_FLAG_SHORT_PRESS_HANDLED_BY_APP) /*| VRMENU_FLAG_BACK_KEY_EXITS_APP*/);

		_OrientationContainer = new UIContainer(gui);
		_OrientationContainer->AddToMenu(_Menu);
		_OrientationContainer->SetLocalRotation(AppState::Conf->Orientation);

		_MainContainer = new UIContainer(gui);
		_MainContainer->AddToMenu(_Menu, _OrientationContainer);
		_MainContainer->SetLocalPosition(Vector3f(0, 0, -1));
		_MainContainer->AddComponent(gazeComponent);
		
		_LabelContainer = new UIContainer(gui);
		_LabelContainer->AddToMenu(_Menu, _MainContainer);

		_SelectorContainer = new UIContainer(gui);
		_SelectorContainer->AddToMenu(_Menu, _MainContainer);
		//_SelectorContainer->SetLocalPosition(Vector3f(0, 0, -1));
		_SelectorContainer->AddComponent(gazeComponent);

		_FillTexture.LoadTextureFromApplicationPackage("assets/fill.png");


		_LocalSrcLabel = new UILabel(gui);
		_LocalSrcLabel->AddToMenu(_Menu, _LabelContainer);
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
		_RemoteSrcLabel->AddToMenu(_Menu, _LabelContainer);
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
		_SelectorContainer->SetLocalPosition(Vector3f(0, -0.3f, 0));
		_Selector->SetOnSelectManga(OnSelectMangaLocal, this);


		//_Selector->SetProvider(_NProvider);
		//_Selector->SetProvider(_LocalMangaProvider);
	}

	void MangaSelectionView::OnLocalCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;

		if (self->_LocalCategoryComponent.Selected == self->_RemoteCategoryComponent.Selected) {
			self->_Mangaroll.Animator.AddTask(
				new AnimatePosition(
				self->_LabelContainer->GetMenuObject(),
				self->_LabelContainer->GetLocalPosition() + Vector3f(0, 0.2f, 0),
				0.2f
			));
		}

		if (!self->_LocalCategoryComponent.Selected) {
			self->_Mangaroll.GetGuiSys().GetSoundEffectPlayer().Play("sv_select");

			self->_LocalCategoryComponent.Selected = true;
			self->_RemoteCategoryComponent.Selected = false;

			self->_Selector->SetProvider(self->_LocalMangaProvider);
		}

	}
	void MangaSelectionView::OnRemoteCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;
		
		if (self->_LocalCategoryComponent.Selected == self->_RemoteCategoryComponent.Selected) {
			self->_Mangaroll.Animator.AddTask(
				new AnimatePosition(
				self->_LabelContainer->GetMenuObject(),
				self->_LabelContainer->GetLocalPosition() + Vector3f(0, 0.2f, 0),
				0.2f
			));
		}

		if (!self->_RemoteCategoryComponent.Selected) {
			self->_Mangaroll.GetGuiSys().GetSoundEffectPlayer().Play("sv_select");

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
	void MangaSelectionView::UpdateGUI() {
		if (_Fader.GetFadeState() != Fader::FADE_NONE) {
			Fader::eFadeState prevState = _Fader.GetFadeState();

			if (_Fader.GetFadeState() == Fader::FADE_IN && !_Menu->IsOpen()) {
				// We're fading in - show menu
				_Menu->Open();
			}

			_Fader.Update(3, Time::Delta);

			if (_Fader.GetFadeState() == Fader::FADE_NONE && prevState == Fader::FADE_OUT) {
				// We're done - close menu
				_Menu->Close();

				if (_CloseRequested) {
					CurViewState = eViewState::VIEWSTATE_CLOSED;
				}
			}

			// Update visual representation
			_MainContainer->SetColor(Vector4f(_Fader.GetFadeAlpha()));

			Vector3f pos = _MainContainer->GetLocalPosition();
			_MainContainer->SetLocalPosition(Vector3f(pos.x, (1 - _Fader.GetFadeAlpha()) * -0.05f, pos.z));
		}

		_OrientationContainer->SetLocalRotation(AppState::Conf->Orientation);

	}

	void MangaSelectionView::OnOpen() {
		_CloseRequested = false;
		CurViewState = eViewState::VIEWSTATE_OPEN;
	
		_Fader.StartFadeIn();
		_AngleOnOpen = _Mangaroll.Carousel.GetAngle();
		_Selector->UpdateGUI();

		Matrix4f orientation;
		orientation.FromQuat(AppState::Conf->Orientation.Inverted());
	}

	void MangaSelectionView::OnClose() {
		_CloseRequested = true;

		_Fader.StartFadeOut();
	}

	bool MangaSelectionView::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) {
		if (eventType == KeyEventType::KEY_EVENT_SHORT_PRESS && keyCode == OVR_KEY_ESCAPE) {
			if (_Selector->CanGoBack()) {
				_Selector->GoBack();
				return true;
			}
			else {
				_Mangaroll.app->StartSystemActivity(PUI_CONFIRM_QUIT);
				return true;
			}
		}
		return false;
	}
	Matrix4f MangaSelectionView::Frame(const VrFrame & vrFrame) {
		UpdateGUI();

		if (vrFrame.Input.buttonPressed & BUTTON_SWIPE_DOWN) {
			if (_Mangaroll.CurrentManga != NULL) {
				_Mangaroll.ShowManga();
			}
		}

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
		, Width(100)
		, Height(150)
		, Border(3)
		, _Cover(NULL)
		, _Preloader(NULL)
		, _CoverTexture(NULL)
		, _Gui(&guiSys)
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
		_Selector = selector;
	}

	MangaWrapper *MangaPanel::GetManga() {
		return _Manga;
	}
	void MangaPanel::UpdateCoverAspectRatio() {
		float realWidth = _Manga->GetCover()->GetWidth();
		float realHeight = _Manga->GetCover()->GetHeight();
		float width = Width - Border;
		float height = Height - Border;

		float aspectTex = realHeight / realWidth;
		float aspectSurf = height / width;

		_Cover->GetMenuObject()->GetSurface(0).SetUVRange(Rect<float>(0, 0, (aspectTex / aspectSurf), 1));
		_Cover->GetMenuObject()->GetSurface(0).RegenerateSurfaceGeometry();
	}

	void MangaPanel::SetManga(MangaWrapper *manga) {
		_Manga = manga;
		
		_Background->SetVisible(true);
		_Background->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, Assets::Instance().Fill, Width, Height);
		_Cover->SetVisible(false);

		//_Title->CalculateTextDimensions();
		if (manga->GetCover()->IsValid()) {
			_Preloader->SetVisible(true);
			_Background->SetColor(Vector4f(0,0,0,1));
			_Cover->SetImage(0, SURFACE_TEXTURE_DIFFUSE, manga->GetCover()->Display(), Width - Border, Height - (float(Border) / Width * Height));
		}
		else {
			_Preloader->SetVisible(false);

			if (manga->GetCover()->IsMoot()) {
				_Background->SetColor(Vector4f(1));
				_Background->SetImage(0, SURFACE_TEXTURE_DIFFUSE, Assets::Instance().Panel, Width, Height);
				//_Background->SetVisible(false);
			}
			else {
				_Background->SetColor(Vector4f(0, 0, 0, 1));
			}
		}
		if (/*manga->IsContainer() || */!manga->GetCover()->IsValid()) {
			_Title->SetVisible(true);
			_Title->SetTextWordWrapped(manga->Name.ToCStr(), _Gui->GetDefaultFont(), VRMenuObject::DEFAULT_TEXEL_SCALE * Width);
			this->GetMenuObject()->SetText("");
		}
		else {
			_Title->SetVisible(false);
			_Title->SetText("");
			this->GetMenuObject()->SetText(manga->Name.ToCStr());
		}

		UpdateProgress();
		/*VRMenuSurface & surf = _Cover->GetMenuObject()->GetSurface(0);
		surf.SetClipUVs();*/
		//_Cover->SetLocalPosition( Vector3f(25,-25,0) );

		//_Cover->SetBorder(UIRectf(10.0f));
		//_Cover->SetLocalPosition(_Cover->GetLocalPosition() + Vector3f(0, 0, 0.05f));
	}

	void MangaPanel::OnClick(void *p) {
		MangaPanel *self = (MangaPanel *)p;

		if (self->_Manga->IsManga()) {
			self->_Selector->SelectManga(self->_Manga->GetManga());
		}
		else {
			self->_Selector->SetProvider(*(self->_Manga->GetContainer()), true);
		}
	}

	void MangaPanel::Update() {
		if (_Preloader->GetVisible()) {
			_Preloader->SetLocalRotation( Quatf(Vector3f(0, 0, 1), -Time::Delta * 5) * _Preloader->GetLocalRotation());

			WARN("MANGA: %p", _Manga);
			if (_Manga->GetCover()->GetState() >= TEXTURE_LOADED) {
				UpdateCoverAspectRatio();

				_Preloader->SetVisible(false);
				_Background->SetColor(Vector4f(0,0,0,1));
				_Cover->SetVisible(true);
			}
		}
	}

	void MangaPanel::UpdateProgress() {
		if (_Manga == NULL || _Manga->IsContainer()) {
			// Remove progress bar
			_Progressbar->SetVisible(false);
		}
		else {
			// Manga is a manga
			MangaInfo info = AppState::Conf->GetProgress(_Manga->GetManga());
			float progress = Alg::Clamp(info.PagesTotal <= 0 ? 0 : float(info.PagesRead) / float(info.PagesTotal), 0.0f, 1.0f);

			_Progressbar->SetVisible(true);
			_Progressbar->GetComponent().SetProgress(progress);
		}
	}

	void MangaPanel::Init(void) {
		this->AddFlags(VRMENUOBJECT_RENDER_HIERARCHY_ORDER);

		_Background = new UIImage(GuiSys);
		_Background->AddToMenu(Menu, this);
		_Background->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, Assets::Instance().Fill, Width, Height);
		//_Background->GetMenuObject()->BuildDrawSurface
		_Background->SetMargin(UIRectf(60.0f));
		_Background->SetColor(Vector4f(0, 0, 0, 1));
		_Background->SetLocalPosition(Vector3f(0, 0, -0.001f));
		
		_Preloader = new UIImage(GuiSys);
		_Preloader->AddToMenu(Menu, this);
		_Preloader->SetColor(Vector4f(1, 1, 1, 0.5f));
		_Preloader->SetImage(0, SURFACE_TEXTURE_DIFFUSE, Assets::Instance().Preloader, Width / 2.0f, Width / 2.0f);
		_Preloader->SetLocalPosition(Vector3f(0, 0, 0.05f));
		_Preloader->SetFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_Preloader->SetVisible(false);

		_Title = new UILabel(GuiSys);
		_Title->AddToMenu(Menu, this);
		_Title->SetText("");
		_Title->SetTextOffset(Vector3f(0, 0, 0.1f));
		_Title->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 0.3));
		_Title->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_Title->SetVisible(false);

		_Progressbar = new UIScrubBar(GuiSys, Width - Border * 2, 10.0f);
		_Progressbar->AddToMenu(Menu, this);
		_Progressbar->SetFillColor(Vector4f(0.6f, 1.0f, 0.6f, 1.0f));
		_Progressbar->SetLocalPosition(Vector3f(0, -Height * 0.3f * VRMenuObject::DEFAULT_TEXEL_SCALE, 0.05f));
		_Progressbar->SetFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_Progressbar->SetVisible(false);


		_Cover = new UIImage(GuiSys);
		_Cover->AddToMenu(Menu, this);
		_Cover->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		
		_Component = new ClickableComponent();
		_Component->SetCallback(OnClick, this);
		AddComponent(new OvrDefaultComponent(
			Vector3f(0, 0, .05f), 1.3f, 0.25f, 0
		));
		AddComponent(_Component);

	}

	//############################################


	MangaSelectorComponent::MangaSelectorComponent(OvrGuiSys &guiSys)
		: VRMenuComponent(
		VRMenuEventFlags_t(VRMENU_EVENT_FRAME_UPDATE) 
		| VRMENU_EVENT_SWIPE_FORWARD 
		| VRMENU_EVENT_SWIPE_BACK 
		| VRMENU_EVENT_FOCUS_GAINED 
		| VRMENU_EVENT_FOCUS_LOST)
		, _Menu(NULL)
		, _Parent(NULL)
		, _Gui(guiSys)
		, _Callback(NULL)
		, _CallbackObject(NULL)
		, _PanelSets()
		, _Front(0)
		, _Back(1)
		, _Transition(0)
		, _Speed(0)
		, _Gravity(0.1f)
		, _Providers()
		, _Arrow()
		, _ArrowLeftTexture()
		, _Fill()
		, _ActivePanel(NULL)
	{

	}


	void MangaSelectorComponent::OnGoBackward(UIButton *, void *p) {
		MangaSelectorComponent *self = (MangaSelectorComponent *)p;
		self->Seek(-1);
	}

	void MangaSelectorComponent::OnGoForward(UIButton *, void *p) {
		MangaSelectorComponent *self = (MangaSelectorComponent *)p;
		self->Seek(1);
	}

	bool MangaSelectorComponent::CanSeek() {
		return CurrentState().Offset + _PanelCount < CurrentProvider()->GetCurrentSize();
	}

	bool MangaSelectorComponent::CanSeekBack() {
		return CurrentState().Offset > 0;
	}


	void MangaSelectorComponent::AddToMenu(UIMenu *menu, UIObject *parent) {
		LOG("[MangaSelectorComponent] Init");

		_Menu = menu;
		_Parent = parent;
		_Parent->AddComponent(this);

		_Containers[0] = new UIContainer(_Gui);
		_Containers[1] = new UIContainer(_Gui);
		_Containers[0]->AddToMenu(_Menu, _Parent);
		_Containers[1]->AddToMenu(_Menu, _Parent);

		_Containers[0]->SetLocalPosition(Vector3f(0, 0, 1));
		_Containers[1]->SetLocalPosition(Vector3f(0, 0, 1));

		_Fill.LoadTextureFromApplicationPackage("assets/fill.png");
		_Title = new UILabel(_Gui);
		_Title->AddToMenu(menu, parent);
		_Title->SetText("");
		_Title->SetTextOffset(Vector3f(0, 0, 0.1f));
		_Title->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 0.3));
		_Title->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_Title->SetVisible(false);
		//_Title->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, _Fill);
		//_Title->SetColor(Vector4f(0, 0, 0, 1));
		

		_Arrow.LoadTextureFromApplicationPackage("assets/arrow.png");
		_ArrowLeftTexture.LoadTextureFromApplicationPackage("assets/arrow_left.png");

		_ArrowLeft = new UIButton(_Gui);
		_ArrowLeft->AddToMenu(_Menu, _Parent);
		_ArrowLeft->SetButtonImages(_ArrowLeftTexture, _ArrowLeftTexture, _ArrowLeftTexture);
		_ArrowLeft->SetButtonColors(Vector4f(0.8f), Vector4f(1), Vector4f(1));
		_ArrowLeft->SetOnClick(OnGoBackward, this);
		_ArrowLeft->SetLocalScale(Vector3f(0.2f, 0.2f, 0.2f));
		_ArrowLeft->SetLocalPosition(Vector3f(-0.3f, 0.45f, 0.1f));
		_ArrowLeft->SetVisible(false);

		_ArrowRight = new UIButton(_Gui);
		_ArrowRight->AddToMenu(_Menu, _Parent);
		_ArrowRight->SetButtonImages(_Arrow, _Arrow, _Arrow);
		_ArrowRight->SetButtonColors(Vector4f(0.8f), Vector4f(1), Vector4f(1));
		_ArrowRight->SetOnClick(OnGoForward, this);
		_ArrowRight->SetLocalScale(Vector3f(0.2f, 0.2f, 0.2f));
		_ArrowRight->SetLocalPosition(Vector3f(0.3f, 0.45f, 0.1f));
		_ArrowRight->SetVisible(false);

		_NoResultMessage = new UILabel(_Gui);
		_NoResultMessage->AddToMenu(_Menu, _Parent);
		_NoResultMessage->SetTextOffset(Vector3f(0, 0, 0.1f));
		_NoResultMessage->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 0.3));
		_NoResultMessage->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_NoResultMessage->SetVisible(false);
		_NoResultMessage->SetLocalPosition(Vector3f(0, 0.25f, 0));

		//_ArrowRight->GetMenuObject()->SetHilightScale(1.1f);

		// Create panels
		_PanelCount = COLS * ROWS;
		for (int y = 0; y < ROWS; y++)
		for (int x = 0; x < COLS; x++)
		{
			_PanelSets[0].PushBack(CreatePanel(x, y, _Containers[0]));
			_PanelSets[1].PushBack(CreatePanel(x, y, _Containers[1]));
		}

		_PanelHeight = _PanelSets[0].Front()->Height * VRMenuObject::DEFAULT_TEXEL_SCALE;
	}

	MangaPanel *MangaSelectorComponent::CreatePanel(int x, int y, UIObject *container) {
		MangaPanel *panel = new MangaPanel(_Gui);
		float xProgress = x / (COLS - 1.0f);
		float rad = (-xProgress * ANGLE + 90 + ANGLE / 2.0f) * Mathf::DegreeToRadFactor;

		panel->AddToMenu(_Menu, container);
		panel->SetSelector(this);
		panel->SetVisible(false);
		panel->SetLocalPosition(Vector3f(
			cos(rad),
			(-y * (panel->Height + MARGIN) + (panel->Height / 2.0f)) * VRMenuObject::DEFAULT_TEXEL_SCALE,
			-sin(rad)
			));

		Vector3f dir = Vector3f(0, 0, 0) - panel->GetLocalPosition();
		dir.y = 0;
		panel->SetLocalRotation(
			Quatf(Vector3f(0, 0, 1), dir.Normalized())
			);
		return panel;
	}

	// Re-Populates panels
	void MangaSelectorComponent::CleanPanels() {
		
		// Clean up
		for (int i = 0; i < _PanelSets[_Back].GetSizeI(); i++) {
			if (_PanelSets[_Back].At(i)->GetVisible()) {
				_PanelSets[_Back].At(i)->SetVisible(false);
				_PanelSets[_Back].At(i)->GetManga()->GetCover()->Hide();
			}
		}
		_Title->SetVisible(false);
	}

	// dir = -1, 0, 1
	void MangaSelectorComponent::Seek(int dir) {
		CurrentState().Offset += dir * _PanelCount;
		_FillCount = 0;

		// Switch Back <-> Front
		_Front = (_Front + 1) % 2;
		_Back = (_Back + 1) % 2;

		// If we aren't already transitioninng, initiate!
		if (_Transition.GetFadeState() == Fader::FADE_NONE) {

			_Transition.SetFadeAlpha(0);
			_Transition.StartFadeIn();
		}
		else {
			// Already animating - just clear the front buffer
			CleanPanels();

			// Switch back, because the back buffer is still moving away
			_Front = (_Front + 1) % 2;
			_Back = (_Back + 1) % 2;
		}


		_FadeDir = dir;
		
	}
	
	void MangaSelectorComponent::UpdatePanels() {
		int count = Alg::Min(CurrentState().Offset + _PanelCount, CurrentProvider()->GetCurrentSize());
		for (int i = CurrentState().Offset + _FillCount; i < count; i++) {
			MangaPanel *panel = _PanelSets[_Front].At(i - CurrentState().Offset);

			panel->SetManga(CurrentProvider()->At(i));
			panel->SetVisible(true);
			_FillCount++;
		}
		for (int i = 0; i < _PanelSets[_Front].GetSizeI(); i++) {
			_PanelSets[_Front][i]->Update();
		}

		_ArrowLeft->SetVisible(CanSeekBack());
		_ArrowRight->SetVisible(CanSeek());
	}


	void MangaSelectorComponent::SetProvider(MangaProvider &provider, bool stack) {
		//if (!IsOperatable())
		//	return;

		if (!stack) {
			// Clean up
			// TODO: Clear memory somehow (be careful when deleting stuff...)
			_Providers.Clear();
		}
		_NoResultMessage->SetVisible(false);
		_Providers.PushBack(ProviderState(&provider, 0));
		Seek(1);
		// Lil hack
		CurrentState().Offset = 0;
	}

	void MangaSelectorComponent::SelectManga(Manga *manga) {
		if (_Callback != NULL) {
			_Callback(manga, _CallbackObject);
		}
	}
	void MangaSelectorComponent::UpdateGUI() {
		for (int i = 0; i < _PanelSets[_Front].GetSizeI(); i++) {
			_PanelSets[_Front][i]->UpdateProgress();
		}
	}

	void MangaSelectorComponent::Update(VRMenuEvent const & evt) {
		// Update index
		if (!CurrentProvider()->IsLoading()) {
			if (CurrentProvider()->HasMore()) {
				if (CurrentProvider()->GetCurrentSize() <= CurrentState().Offset + _PanelCount * 2) {
					CurrentProvider()->LoadMore();
				}
			}
			else {
				// Outloaded
				if ( CurrentProvider()->GetCurrentSize() == 0 && !_NoResultMessage->GetVisible()) {
					_NoResultMessage->SetVisible(true);
					_NoResultMessage->SetTextWordWrapped(CurrentProvider()->GetNoResultMessage().ToCStr(), _Gui.GetDefaultFont(), 0.8f);
				}
			}
		}

		UpdatePanels();

		// ### Process Label ###
		if (_ActivePanel != NULL && _Transition.GetFadeState() != Fader::FADE_IN) {
			Vector3f pos = _ActivePanel->GetLocalPosition();
			pos.z += 1; // Parent container is positioned a little more toward the player
			pos.y = _ActivePanel->GetLocalPosition().y - _PanelHeight / 2;

			float t = Time::Delta * 10;
			_Title->SetLocalPose(
				_Title->GetLocalRotation().Nlerp(_ActivePanel->GetLocalRotation(), 1 - t),
				_Title->GetLocalPosition().Lerp(pos, t)
			);
		}
		else {
			if (_Title->GetVisible() && Time::Elapsed - _FocusLostTime > 1.0f) {
				float alpha = 1 - (Time::Elapsed - _FocusLostTime - 1);
				if (alpha <= 0) {
					_Title->SetVisible(false);
				}
				else {
					_Title->SetTextColor(Vector4f(alpha));
				}
			}
		}

		// ### Process Transition ###
		if (_Transition.GetFadeState() == Fader::FADE_IN) {
			_Transition.Update(ProviderCaughtUp() ? 1.0f : 0.1f, Time::Delta);

			// Apply values (Front is new, back is old)
			_Containers[_Front]->SetColor(Vector4f(_Transition.GetFadeAlpha()));
			_Containers[_Back]->SetColor(Vector4f(1 - _Transition.GetFadeAlpha()));

			float angle = (ANGLE + (float(ANGLE) / COLS)) / 180.0f * Mathf::Pi;

			_Containers[_Front]->SetLocalRotation(Quatf(Vector3f(0, 1, 0), (1 - _Transition.GetFadeAlpha()) * angle * -_FadeDir));
			_Containers[_Back]->SetLocalRotation(Quatf(Vector3f(0, 1, 0), _Transition.GetFadeAlpha() * angle * _FadeDir));

			if (_Transition.GetFadeState() == Fader::FADE_NONE) {
				CleanPanels();
			}
		}
	}

	bool MangaSelectorComponent::ProviderCaughtUp() {
		return CurrentState().Offset < CurrentProvider()->GetCurrentSize() || (!CurrentProvider()->HasMore() && !CurrentProvider()->IsLoading());
	}
	
	eMsgStatus MangaSelectorComponent::OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		if (_Providers.GetSizeI() == 0) return MSG_STATUS_ALIVE;

		if (event.EventType == VRMENU_EVENT_FRAME_UPDATE) {
			Update(event);
			return MSG_STATUS_ALIVE;
		}
		
		// No input in this case
		if (_Transition.GetFadeState() == Fader::FADE_IN) {
			return MSG_STATUS_ALIVE;
		}

		switch (event.EventType) {
		case VRMENU_EVENT_FOCUS_GAINED:
			_ActivePanel = _Gui.GetVRMenuMgr().ToObject(_Gui.GetVRMenuMgr().ToObject(event.TargetHandle)->GetParentHandle());
			if (_ActivePanel->GetText() != "Container" && !_ActivePanel->GetText().IsEmpty()) {
				_Title->SetVisible(true);
				_Title->SetTextColor(Vector4f(1));
				_Title->SetTextWordWrapped(_ActivePanel->GetText().ToCStr(), _Gui.GetDefaultFont(), 0.6f);

				_Title->CalculateTextDimensions();
			}
			else {
				_ActivePanel = NULL;
			}

			//LOG("TEXT SIZE:_ %.2f / %.2f", _Title->GetRect().GetWidth(), _Title->GetRect().GetHeight());
			break;
		case VRMENU_EVENT_SWIPE_FORWARD:
			if (CanSeek()) {
				OnGoForward(NULL, this);
			}
			return MSG_STATUS_CONSUMED;
			break;
		case VRMENU_EVENT_SWIPE_BACK:
			if (CanSeekBack()) {
				OnGoBackward(NULL, this);
			}
			return MSG_STATUS_CONSUMED;
			break;
		case VRMENU_EVENT_FOCUS_LOST:
			//_Title->SetVisible(false);
			_FocusLostTime = Time::Elapsed;
			_ActivePanel = NULL;
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
			guiSys.GetSoundEffectPlayer().Play("sv_focusgained");
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