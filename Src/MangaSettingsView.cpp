#include "MangaSettingsView.h"
#include "Mangaroll.h"
#include "GLES3\gl3_loader.h"
#include "DefaultComponent.h"
#include "VRMenuMgr.h"
#include "Helpers.h"
#include "Kernel\OVR_String_Utils.h"

namespace OvrMangaroll {

	float PixelScale(const float x)
	{
		return x * VRMenuObject::DEFAULT_TEXEL_SCALE;
	}
	Vector3f PixelPos(const float x, const float y, const float z)
	{
		return Vector3f(PixelScale(x), PixelScale(y), PixelScale(z));
	}

	void OnText(ScrubBarComponent *button, void *object, UILabel *label, int progress) {
		label->SetText(String::Format("Page %d", progress + 1));
	}

	void OnPageProgressClick(ScrubBarComponent *slider, void *object, float progress) {
		((MangaSettingsView *)object)->SetPageProgress(progress);
	}


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

	MangaSettingsView::MangaSettingsView(Mangaroll *app) : 
		View("MangaSettingsView")
		, _Mangaroll(app)
		, _Menu(NULL)
		, _CenterContainer(NULL)
		, _LeftContainer(NULL)
		, _RightContainer(NULL)
		, _TitleLabel(NULL)
		, _PageLabel(NULL)
		, _GammaSlider(NULL)
		, _ContrastSlider(NULL)
		, _BrightnessSlider(NULL)
		, _Fader(0)
		, _ProgressComponent()
		, _PageSeekLabel(NULL)
		, _ProgressBG(NULL)
		, _ProgressFG(NULL)
		, _ProgressBGTexture()
		, _ProgressFGTexture()
	{
	}

	MangaSettingsView::~MangaSettingsView() {

	}

	bool MangaSettingsView::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) {
		if (keyCode == OVR_KEY_BACK && eventType == KeyEventType::KEY_EVENT_SHORT_PRESS) {
			if (_Menu->IsOpen()) {
				HideGUI();
			}
			else {
				ShowGUI();
			}
			return true;
		}
		return false;
	}
	Matrix4f MangaSettingsView::Frame(const VrFrame & vrFrame) {		
		UpdateMenuState();

		return _Mangaroll->Carousel.Frame(vrFrame);
	}
	Matrix4f MangaSettingsView::GetEyeViewMatrix(const int eye) const {
		return _Mangaroll->Carousel.GetEyeViewMatrix(eye);
	}
	Matrix4f MangaSettingsView::GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const {
		return _Mangaroll->Carousel.GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
	}
	Matrix4f MangaSettingsView::DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) {

		return _Mangaroll->Carousel.DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
	}

	void MangaSettingsView::OneTimeInit(const char * launchIntent) {
		// BUILD GUI
		const Quatf forward(Vector3f(0.0f, 1.0f, 0.0f), 0.0f);

		OvrGuiSys &gui = _Mangaroll->GetGuiSys();
		_Menu = new UIMenu(gui);
		_Menu->Create("MangaMenu");
		_Menu->SetFlags(VRMenuFlags_t(VRMENU_FLAG_PLACE_ON_HORIZON) | VRMENU_FLAG_SHORT_PRESS_HANDLED_BY_APP);

		_CenterContainer = new UIContainer(gui);
		_CenterContainer->AddToMenu(_Menu);
		_CenterContainer->SetLocalPose(forward, Vector3f(0, 0, -.9f));
		_CenterContainer->SetColor(Vector4f(1, 1, 1, 0.5f));

		_TitleLabel = new UILabel(gui);
		_TitleLabel->AddToMenu(_Menu, _CenterContainer);
		// It's unknown what these parms actually do... but well, it works. "alpha" seems to control the thickness of the outline
		_TitleLabel->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 1));
		_TitleLabel->SetLocalPose(forward, Vector3f(0, 0.1f, 0));
		_TitleLabel->SetFontScale(0.7f);
		_TitleLabel->SetText("");

		_PageLabel = new UILabel(gui);
		_PageLabel->AddToMenu(_Menu, _CenterContainer);
		_PageLabel->SetFontParms(VRMenuFontParms(true, true, false, false, true, 0.5f, 0.4f, 1));
		_PageLabel->SetLocalPose(forward, Vector3f(0, 0, 0));
		_PageLabel->SetFontScale(0.5f);
		_PageLabel->SetText("");

		GazeUpdaterComponent *component = new GazeUpdaterComponent();
		component->HandlesEvent(VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED) /*| VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_FRAME_UPDATE*/);
		_CenterContainer->AddComponent(component);

		_ProgressBGTexture.LoadTextureFromApplicationPackage("assets/progress_bg.png");
		_ProgressFGTexture.LoadTextureFromApplicationPackage("assets/fill.png");


		float barWidth = 7 * 30;
		float barHeight = 30;

		_ProgressBG = new UIImage(gui);
		_ProgressBG->AddToMenu(_Menu, _CenterContainer);
		_ProgressBG->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, _ProgressBGTexture, barWidth, barHeight);
		_ProgressBG->SetLocalPose(forward, Vector3f(0, -0.05f, 0));
		_ProgressBG->AddComponent(&_ProgressComponent);
		_ProgressBG->GetMenuObject()->AddFlags(VRMENUOBJECT_RENDER_HIERARCHY_ORDER);

		_ProgressFG = new UIImage(gui);
		_ProgressFG->AddToMenu(_Menu, _ProgressBG);
		_ProgressFG->SetLocalPosition(PixelPos(-3, 0, 1));
		_ProgressFG->SetImage(0, SURFACE_TEXTURE_ADDITIVE, _ProgressFGTexture, 0, 0);
		_ProgressFG->GetMenuObject()->AddFlags(VRMenuObjectFlags_t(VRMENUOBJECT_DONT_HIT_ALL));


		_PageSeekLabel = new UILabel(gui);
		_PageSeekLabel->AddToMenu(_Menu, _ProgressBG);
		_PageSeekLabel->GetMenuObject()->AddFlags(VRMENUOBJECT_DONT_HIT_ALL);
		_PageSeekLabel->SetLocalPose(forward, Vector3f(0, 0, 0));
		_PageSeekLabel->SetFontScale(0.5f);
		_PageSeekLabel->SetText("");
		_PageSeekLabel->SetTextOffset(Vector3f(0, -0.1f, 0));

		_ProgressComponent.SetWidgets(_Menu, _ProgressBG, _ProgressFG, _PageLabel, _PageSeekLabel, barWidth - 6, barHeight - 3);
		_ProgressComponent.SetOnText(OnText, this);
		_ProgressComponent.SetProgress(0.5f);
		_ProgressComponent.SetOnClick(OnPageProgressClick, this);
		
		
		//_ProgressBar->AddToMenu(_Menu, true, true, _CenterContainer);
		//_ProgressBar->SetLocalPose(forward, Vector3f(0, 0, 0));
		//_ProgressBar->SetColor(Vector4f(1, 1, 1, 1));
		//_ProgressBar->SetProgress(0.0f);
		//_ProgressBar->SetDimensions(Vector2f(5, 10));
		//_ProgressBar->SetBorder(UIRectf(10, 10, 10, 10));

		/*_GammaSlider = new UIDiscreteSlider(gui);
		_GammaSlider->AddToMenu(_Menu, _CenterContainer);
		_GammaSlider->AddCells(10, 0, 0);
		_GammaSlider->SetLocalPose(forward, Vector3f(0, 0, 0));*/
		//_GammaSlider->SetColor(Vector4f(1, 1, 1, 1));

		//gui.GetGazeCursor().UpdateDistance(0.7f, eGazeCursorStateType::CURSOR_STATE_NORMAL);
	}

	void MangaSettingsView::SetPageProgress(float progress) {
		_ProgressComponent.SetProgress(progress);
		_Mangaroll->CurrentManga.SetProgress(progress * _Mangaroll->CurrentManga.GetCount());
	}

	void MangaSettingsView::OneTimeShutdown() {
	}

	void MangaSettingsView::OnOpen() {
		CurViewState = eViewState::VIEWSTATE_OPEN;
		
	}
	void MangaSettingsView::OnClose() {
		CurViewState = eViewState::VIEWSTATE_CLOSED;

		_Menu->Close();

	}

	void MangaSettingsView::UpdateMenuState(void) {
		
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
			}

			// Update visual representation
			_CenterContainer->SetColor(Vector4f(_Fader.GetFadeAlpha()));

			Vector3f pos = _CenterContainer->GetLocalPosition();
			_CenterContainer->SetLocalPosition(Vector3f(pos.x, (1 - _Fader.GetFadeAlpha()) * -0.05f, pos.z));
		}
	}

	void MangaSettingsView::ShowGUI(void) {
		// Update values
		_Menu->GetVRMenu()->RepositionMenu(GetEyeViewMatrix(0));
		_ProgressComponent.SetMax(_Mangaroll->CurrentManga.GetCount());
		_ProgressComponent.SetProgress(_Mangaroll->CurrentManga.GetProgress() / (_Mangaroll->CurrentManga.GetCount() - 1.0f));
		_PageLabel->SetText(String::Format("Page %d", _Mangaroll->CurrentManga.GetProgress() + 1));
		_TitleLabel->SetText(_Mangaroll->CurrentManga.Name);


		_Fader.StartFadeIn();
		_Mangaroll->Carousel.MoveOut();
		_Mangaroll->GetGuiSys().GetGazeCursor().ShowCursor();
	}

	void MangaSettingsView::HideGUI(void) {
		_Fader.StartFadeOut();
		_Mangaroll->Carousel.MoveIn();
		_Mangaroll->GetGuiSys().GetGazeCursor().HideCursor();
	}



//##################################################################



	ScrubBarComponent::ScrubBarComponent() :
		VRMenuComponent(VRMenuEventFlags_t(VRMENU_EVENT_TOUCH_DOWN) |
		VRMENU_EVENT_TOUCH_DOWN |
		VRMENU_EVENT_FRAME_UPDATE |
		VRMENU_EVENT_FOCUS_GAINED |
		VRMENU_EVENT_FOCUS_LOST),
		HasFocus(false),
		TouchDown(false),
		Progress(0.0f),
		Max(0),
		Menu(NULL),
		Background(NULL),
		ScrubBar(NULL),
		CurrentTime(NULL),
		SeekTime(NULL),
		OnClickFunction(NULL),
		OnSetTextFunction(NULL),
		OnClickObject(NULL),
		OnTextObject(NULL)
	{
	}

	void ScrubBarComponent::SetMax(const int max)
	{
		Max = max;

		SetProgress(Progress);
	}

	int ScrubBarComponent::GetMax(void) {
		return Max;
	}

	void ScrubBarComponent::SetOnClick(void(*callback)(ScrubBarComponent *, void *, float), void *object)
	{
		OnClickFunction = callback;
		OnClickObject = object;
	}

	void ScrubBarComponent::SetOnText(void(*callback)(ScrubBarComponent *, void*, UILabel *label, int progress), void *object) {
		OnSetTextFunction = callback;
		OnTextObject = object;
	}

	void ScrubBarComponent::SetWidgets(UIMenu * menu, UIObject *background, UIObject *scrubBar, UILabel *currentTime, UILabel *seekTime, const int scrubBarWidth, const int scrubBarHeight)
	{
		Menu = menu;
		Background = background;
		ScrubBar = scrubBar;
		CurrentTime = currentTime;
		SeekTime = seekTime;
		ScrubBarWidth = scrubBarWidth;
		ScrubBarHeight = scrubBarHeight;

		SeekTime->SetVisible(false);
	}

	void ScrubBarComponent::SetProgress(const float progress)
	{
		Progress = progress;
		const float seekwidth = ScrubBarWidth * progress;

		Vector3f pos = ScrubBar->GetLocalPosition();
		pos.x = PixelScale((ScrubBarWidth - seekwidth) * -0.5f);
		ScrubBar->SetLocalPosition(pos);
		ScrubBar->SetSurfaceDims(0, Vector2f(seekwidth, ScrubBarHeight));
		ScrubBar->RegenerateSurfaceGeometry(0, false);

		/*pos = CurrentTime->GetLocalPosition();
		pos.x = PixelScale(ScrubBarWidth * -0.5f + seekwidth);
		CurrentTime->SetLocalPosition(pos);*/

		if (OnSetTextFunction != NULL) {
			(OnSetTextFunction)(this, OnTextObject, CurrentTime, Max * progress);
		}
		//SetTimeText(CurrentTime, Max * progress);
	}

	//void ScrubBarComponent::SetTimeText(UILabel *label, const int time)
	//{
	//	int seconds = time / 1000;
	//	int minutes = seconds / 60;
	//	int hours = minutes / 60;
	//	seconds = seconds % 60;
	//	minutes = minutes % 60;

	//	if (hours > 0)
	//	{
	//		label->SetText(StringUtils::Va("%d:%02d:%02d", hours, minutes, seconds));
	//	}
	//	else if (minutes > 0)
	//	{
	//		label->SetText(StringUtils::Va("%d:%02d", minutes, seconds));
	//	}
	//	else
	//	{
	//		label->SetText(StringUtils::Va("0:%02d", seconds));
	//	}
	//}

	eMsgStatus ScrubBarComponent::OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event)
	{
		switch (event.EventType)
		{
		case VRMENU_EVENT_FOCUS_GAINED:
			HasFocus = true;
			return MSG_STATUS_ALIVE;

		case VRMENU_EVENT_FOCUS_LOST:
			HasFocus = false;
			return MSG_STATUS_ALIVE;

		case VRMENU_EVENT_TOUCH_DOWN:
			TouchDown = true;
			OnClick(guiSys, vrFrame, event);
			return MSG_STATUS_ALIVE;

		case VRMENU_EVENT_FRAME_UPDATE:
			return OnFrame(guiSys, vrFrame, self, event);

		default:
			OVR_ASSERT(!"Event flags mismatch!");
			return MSG_STATUS_ALIVE;
		}
	}

	eMsgStatus ScrubBarComponent::OnFrame(OvrGuiSys & guiSys, VrFrame const & vrFrame,
		VRMenuObject * self, VRMenuEvent const & event)
	{
		if (TouchDown)
		{
			if ((vrFrame.Input.buttonState & (BUTTON_A | BUTTON_TOUCH)) != 0)
			{
				OnClick(guiSys, vrFrame, event);
			}
			else
			{
				TouchDown = false;
			}
		}

		SeekTime->SetVisible(HasFocus);
		if (HasFocus)
		{
			Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

			// move hit position into local space
			const Posef modelPose = Menu->GetVRMenu()->GetMenuPose() * Background->GetWorldPose();
			Vector3f localHit = modelPose.Orientation.Inverted().Rotate(hitPos - modelPose.Position);

			//WARN("%.2f | %.2f | %.2f", modelPose.Orientation.x, modelPose.Orientation.y, modelPose.Orientation.z);
			Bounds3f bounds = Background->GetMenuObject()->GetLocalBounds(guiSys.GetDefaultFont()) * Background->GetParent()->GetWorldScale();
			const float progress = (localHit.x - bounds.GetMins().x) / bounds.GetSize().x;

			if ((progress >= 0.0f) && (progress <= 1.0f))
			{
				const float seekwidth = ScrubBarWidth * progress;
				Vector3f pos = SeekTime->GetLocalPosition();
				pos.x = PixelScale(ScrubBarWidth * -0.5f + seekwidth);
				SeekTime->SetLocalPosition(pos);

				if (OnSetTextFunction != NULL) {
					(OnSetTextFunction)(this, OnTextObject, SeekTime, Max * progress);
				}
				//SetTimeText(SeekTime, Max * progress);
			}
		}

		return MSG_STATUS_ALIVE;
	}

	void ScrubBarComponent::OnClick(OvrGuiSys & guiSys, VrFrame const & vrFrame, VRMenuEvent const & event)
	{
		if (OnClickFunction == NULL)
		{
			return;
		}

		Vector3f hitPos = event.HitResult.RayStart + event.HitResult.RayDir * event.HitResult.t;

		// move hit position into local space
		const Posef modelPose = Menu->GetVRMenu()->GetMenuPose() * Background->GetWorldPose();
		Vector3f localHit = modelPose.Orientation.Inverted().Rotate(hitPos - modelPose.Position);

		Bounds3f bounds = Background->GetMenuObject()->GetLocalBounds(guiSys.GetDefaultFont()) * Background->GetParent()->GetWorldScale();
		const float progress = (localHit.x - bounds.GetMins().x) / bounds.GetSize().x;
		if ((progress >= 0.0f) && (progress <= 1.0f))
		{
			(*OnClickFunction)(this, OnClickObject, progress);
		}
	}



}