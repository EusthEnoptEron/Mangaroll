#include "MangaSettingsView.h"
#include "Mangaroll.h"
#include "GLES3\gl3_loader.h"
#include "DefaultComponent.h"
#include "VRMenuMgr.h"

namespace OvrMangaroll {

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
				return eMsgStatus::MSG_STATUS_CONSUMED;

				break;
			case VRMENU_EVENT_FOCUS_LOST:
				return eMsgStatus::MSG_STATUS_CONSUMED;
				break;
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
		, _ProgressBar(NULL)
		, _GammaSlider(NULL)
		, _ContrastSlider(NULL)
		, _BrightnessSlider(NULL)
	{
	}

	MangaSettingsView::~MangaSettingsView() {

	}

	bool MangaSettingsView::OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) {
		if (keyCode == OVR_KEY_BACK && eventType == KeyEventType::KEY_EVENT_SHORT_PRESS) {
			if (_Menu->IsOpen()) {
				_Mangaroll->GetGuiSys().GetGazeCursor().HideCursor();
				_Menu->Close();
			}
			else {
				_Mangaroll->GetGuiSys().GetGazeCursor().ShowCursor();
				_Menu->Open();
				_Menu->GetVRMenu()->RepositionMenu(GetEyeViewMatrix(0));
				_ProgressBar->SetProgress(_Mangaroll->CurrentManga.GetProgress() / (_Mangaroll->CurrentManga.GetCount() - 1.0f)  );
				
				
				_PageLabel->SetText(String::Format("Page %d", _Mangaroll->CurrentManga.GetProgress() + 1));
				_TitleLabel->SetText(_Mangaroll->CurrentManga.Name);
			}
			return true;
		}
		return false;
	}
	Matrix4f MangaSettingsView::Frame(const VrFrame & vrFrame) {		
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
		_Menu->SetFlags(VRMENU_FLAG_PLACE_ON_HORIZON);

		_CenterContainer = new UIContainer(gui);
		_CenterContainer->AddToMenu(_Menu);
		_CenterContainer->SetLocalPose(forward, Vector3f(0, 0, -.9f));

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
		//component->HandlesEvent(VRMenuEventFlags_t(VRMENU_EVENT_FOCUS_GAINED) | VRMENU_EVENT_FOCUS_LOST | VRMENU_EVENT_FRAME_UPDATE);
		_CenterContainer->AddComponent(component);

		UITexture *texture = new UITexture();
		texture->LoadTextureFromApplicationPackage("assets/progress_bg.png");

		UIImage *ProgressBar = new UIImage(gui);
		ProgressBar->AddToMenu(_Menu, _CenterContainer);
		ProgressBar->SetImage(0, eSurfaceTextureType::SURFACE_TEXTURE_DIFFUSE, *texture, 7 * 30, 30);
		ProgressBar->SetLocalPose(forward, Vector3f(0, -0.05f, 0));
		
		_ProgressBar = new UIProgressBar(gui);
		
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

	void MangaSettingsView::OneTimeShutdown() {
	}

	void MangaSettingsView::OnOpen() {
		CurViewState = eViewState::VIEWSTATE_OPEN;
		
	}
	void MangaSettingsView::OnClose() {
		CurViewState = eViewState::VIEWSTATE_CLOSED;

		_Menu->Close();

	}

}