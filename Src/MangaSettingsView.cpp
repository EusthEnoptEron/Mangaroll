#include "MangaSettingsView.h"
#include "Mangaroll.h"
#include "GLES3\gl3_loader.h"

namespace OvrMangaroll {

	MangaSettingsView::MangaSettingsView(Mangaroll *app) : 
		View("MangaSettingsView")
		, _Manga(app)
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
		return false;
	}
	Matrix4f MangaSettingsView::Frame(const VrFrame & vrFrame) {
		return _Manga->Carousel.Frame(vrFrame);
	}
	Matrix4f MangaSettingsView::GetEyeViewMatrix(const int eye) const {
		return _Manga->Carousel.GetEyeViewMatrix(eye);
	}
	Matrix4f MangaSettingsView::GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const {
		return _Manga->Carousel.GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
	}
	Matrix4f MangaSettingsView::DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) {

		return _Manga->Carousel.DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
	}

	void MangaSettingsView::OneTimeInit(const char * launchIntent) {

	}
	void MangaSettingsView::OneTimeShutdown() {
	}

	void MangaSettingsView::OnOpen() {
		CurViewState = eViewState::VIEWSTATE_OPEN;
	}
	void MangaSettingsView::OnClose() {
		CurViewState = eViewState::VIEWSTATE_CLOSED;
	}
}