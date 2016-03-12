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
		, _NProvider("http://192.168.1.39:3000/browse/%1$d", "http://192.168.1.39:3000/show/%1$d")
		, _RemoteProviders()
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

		// LOAD CONFIGURED REMOTE SITES
		const OvrStoragePaths & paths = AppState::Instance->GetStoragePaths();

		Array<String> SearchPaths;
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

		for (int i = 0; i < SearchPaths.GetSizeI(); i++) {
			const char *path = (SearchPaths[i] + "Manga/services.json").ToCStr();
			if (FileExists(path)) {
				JSON *jsonFile = JSON::Load(path);
				if (jsonFile != NULL) {
					JsonReader serviceReader(jsonFile);
					if (serviceReader.IsValid() && serviceReader.IsArray()) {
						while (!serviceReader.IsEndOfArray()) {
							JsonReader elementReader(serviceReader.GetNextArrayElement());
							if (elementReader.IsValid() && elementReader.IsObject()) {
								RemoteMangaProvider *provider = new RemoteMangaProvider(
									elementReader.GetChildStringByName("browseUrl"),
									elementReader.GetChildStringByName("showUrl")
								);
								provider->Name = elementReader.GetChildStringByName("name");

								_RemoteProviders.PushBack(provider);
							}
						}
					}
					delete jsonFile;
				}
				//break;
			}
		}

		LOG("Services found: %d", _RemoteProviders.GetSizeI());

		//_Selector->SetProvider(_NProvider);
		//_Selector->SetProvider(_LocalMangaProvider);
	}

	void MangaSelectionView::OnLocalCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;

		if (self->_LocalCategoryComponent.Selected == self->_RemoteCategoryComponent.Selected) {
			WARN("ANIMATE");
			self->_Mangaroll.Animator.AddTask(
				new AnimatePosition(
				self->_MainContainer->GetMenuObject(),
				self->_MainContainer->GetLocalPosition() + Vector3f(0, 0.2f, 0),
				0.2f
			));
		}

		if (!self->_LocalCategoryComponent.Selected) {
			self->_LocalCategoryComponent.Selected = true;
			self->_RemoteCategoryComponent.Selected = false;

			self->_Selector->SetProvider(self->_LocalMangaProvider);
		}

	}
	void MangaSelectionView::OnRemoteCategory(void *p) {
		MangaSelectionView *self = (MangaSelectionView *)p;
		
		if (self->_LocalCategoryComponent.Selected == self->_RemoteCategoryComponent.Selected) {
			WARN("ANIMATE");
			self->_Mangaroll.Animator.AddTask(
				new AnimatePosition(
				self->_MainContainer->GetMenuObject(),
				self->_MainContainer->GetLocalPosition() + Vector3f(0, 0.2f, 0),
				0.2f
			));
		}

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
	, Width(100)
	, Height(150)
	, Border(10)
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
		_Selector = selector;
	}

	Manga *MangaPanel::GetManga() {
		return _Manga;
	}
	void MangaPanel::SetManga(Manga *manga) {
		_Manga = manga;
		this->GetMenuObject()->SetText(manga->Name.ToCStr());

		//_Title->CalculateTextDimensions();

		_Cover->SetImage(0, SURFACE_TEXTURE_DIFFUSE, manga->GetCover()->Display(), Width-Border, Height - (float(Border) / Width * Height));
		/*VRMenuSurface & surf = _Cover->GetMenuObject()->GetSurface(0);
		surf.SetClipUVs();*/
		//_Cover->SetLocalPosition( Vector3f(25,-25,0) );

		//_Cover->SetBorder(UIRectf(10.0f));
		//_Cover->SetLocalPosition(_Cover->GetLocalPosition() + Vector3f(0, 0, 0.05f));

	}

	void MangaPanel::OnClick(void *p) {
		MangaPanel *self = (MangaPanel *)p;

		self->_Selector->SelectManga(self->_Manga);
	}

	void MangaPanel::Init(void) {
		UITexture *_BGTexture = new UITexture();
		_BGTexture->LoadTextureFromApplicationPackage("assets/fill.png");
		this->AddFlags(VRMENUOBJECT_RENDER_HIERARCHY_ORDER);

		_Background = new UIImage(GuiSys);
		_Background->AddToMenu(Menu, this);
		_Background->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, *_BGTexture, Width, Height);
		//_Background->GetMenuObject()->BuildDrawSurface
		_Background->SetMargin(UIRectf(60.0f));
		_Background->SetColor(Vector4f(0, 0, 0, 1));
		_Background->SetLocalPosition(Vector3f(0, 0, -0.001f));
		
		_Cover = new UIImage(GuiSys);
		_Cover->AddToMenu(Menu, this);
		_Cover->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		
		_Component = new ClickableComponent();
		_Component->SetCallback(OnClick, this);
		AddComponent(new OvrDefaultComponent(
			Vector3f(0, 0, .05f), 1.4f, 0.25f, 0
		));
		AddComponent(_Component);

	}

	//############################################


	MangaSelectorComponent::MangaSelectorComponent(OvrGuiSys &guiSys)
		: VRMenuComponent(
		VRMenuEventFlags_t(VRMENU_EVENT_FRAME_UPDATE) | VRMENU_EVENT_SWIPE_FORWARD | VRMENU_EVENT_SWIPE_BACK | VRMENU_EVENT_FOCUS_GAINED | VRMENU_EVENT_FOCUS_LOST)
		, _Menu(NULL)
		, _Parent(NULL)
		, _Gui(guiSys)
		, _Callback(NULL)
		, _CallbackObject(NULL)
		, _PanelSets()
		, _Front(0)
		, _Back(1)
		, _Transition(0)
		, _Index(0)
		, _Speed(0)
		, _Gravity(0.1f)
		, _Provider(NULL)
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
		return _Index + _PanelCount < _Provider->GetCurrentSize();
	}

	bool MangaSelectorComponent::CanSeekBack() {
		return _Index > 0;
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
		_Index += dir * _PanelCount;
		_FillCount = 0;
		_Front = (_Front + 1) % 2;
		_Back = (_Back + 1) % 2;
		_FadeDir = dir;
		_Transition.SetFadeAlpha(0);
		_Transition.StartFadeIn();
	}
	
	void MangaSelectorComponent::UpdatePanels() {
		int count = Alg::Min(_Index + _PanelCount, _Provider->GetCurrentSize());
		for (int i = _Index + _FillCount; i < count; i++) {
			MangaPanel *panel = _PanelSets[_Front].At(i - _Index);

			panel->SetManga(_Provider->At(i));
			panel->SetVisible(true);
			_FillCount++;
		}

		_ArrowLeft->SetVisible(CanSeekBack());
		_ArrowRight->SetVisible(CanSeek());
	}


	void MangaSelectorComponent::SetProvider(MangaProvider &provider) {
		_Provider = &provider;
		_Index = 0;
		
		Seek(0);
		CleanPanels();
	}

	void MangaSelectorComponent::SelectManga(Manga *manga) {
		if (_Callback != NULL) {
			_Callback(manga, _CallbackObject);
		}
	}

	void MangaSelectorComponent::Update(VRMenuEvent const & evt) {
		// Update index
		if (_Provider != NULL) {
			if (!_Provider->IsLoading() && _Provider->HasMore()) {
				if (_Provider->GetCurrentSize() <= _Index + _PanelCount) {
					_Provider->LoadMore();
				}
			}
		}
		UpdatePanels();

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

		if (_Transition.GetFadeState() == Fader::FADE_IN) {
			_Transition.Update(1.0f, Time::Delta);

			// Apply values
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
	
	eMsgStatus MangaSelectorComponent::OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event) {
		if (_Provider == NULL) return MSG_STATUS_ALIVE;

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
			if (_ActivePanel->GetText() != "Container") {
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