#include "MangaSettingsView.h"
#include "Mangaroll.h"
#include "GLES3\gl3_loader.h"

namespace OvrMangaroll {

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
		_CenterContainer->SetLocalPose(forward, Vector3f(0, 0, -.8f));
		
		_TitleLabel = new UILabel(gui);
		_TitleLabel->AddToMenu(_Menu, _CenterContainer);
		_TitleLabel->SetLocalPose(forward, Vector3f(0, 0, 0));
		_TitleLabel->SetFontScale(0.5f);
		_TitleLabel->SetText("TEST");

		UITexture *texture = new UITexture();
		texture->LoadTextureFromApplicationPackage("assets/img_progressbar_background.png");

		_ProgressBar = new UIProgressBar(gui);
		_ProgressBar->AddToMenu(_Menu, true, true, _CenterContainer);
		_ProgressBar->SetLocalPose(forward, Vector3f(0, 0, 0));
		_ProgressBar->SetColor(Vector4f(1, 1, 1, 1));
		_ProgressBar->SetProgress(0.0f);

		/*_GammaSlider = new UIDiscreteSlider(gui);
		_GammaSlider->AddToMenu(_Menu, _CenterContainer);
		_GammaSlider->AddCells(10, 0, 0);
		_GammaSlider->SetLocalPose(forward, Vector3f(0, 0, 0));*/
		//_GammaSlider->SetColor(Vector4f(1, 1, 1, 1));
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