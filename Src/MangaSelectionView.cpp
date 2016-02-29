#include "MangaSelectionView.h"
#include "Mangaroll.h"

namespace OvrMangaroll {

	MangaSelectionView::MangaSelectionView(Mangaroll &app)
		: View("MangaSelectionView")
		, _Mangaroll(app)
		, _Menu(NULL)
		, _MainContainer(NULL)
		, _LocalSrcLabel(NULL)
		, _RemoteSrcLabel(NULL)
		
	{

	}

	MangaSelectionView::~MangaSelectionView() {

	}

	void MangaSelectionView::OneTimeInit(const char *launchIntent) {
		OvrGuiSys &gui = _Mangaroll.GetGuiSys();
		_Menu = new UIMenu(gui);
		_Menu->Create("MangaSelectionMenu");
		_Menu->SetFlags(VRMenuFlags_t(VRMENU_FLAG_PLACE_ON_HORIZON) | VRMENU_FLAG_BACK_KEY_EXITS_APP);

		_MainContainer = new UIContainer(gui);
		_MainContainer->AddToMenu(_Menu);
		_MainContainer->SetLocalPosition(Vector3f(0, 0, -1));

		UITexture *fill = new UITexture();
		fill->LoadTextureFromApplicationPackage("assets/fill.png");

		_LocalSrcLabel = new UILabel(gui);
		_LocalSrcLabel->AddToMenu(_Menu, _MainContainer);
		_LocalSrcLabel->SetLocalPosition(Vector3f(0.1f, 0, 0));
		_LocalSrcLabel->SetText("Local");
		_LocalSrcLabel->SetFontScale(0.5f);
		_LocalSrcLabel->CalculateTextDimensions();

		_RemoteSrcLabel = new UILabel(gui);
		_RemoteSrcLabel->AddToMenu(_Menu, _MainContainer);
		_RemoteSrcLabel->SetLocalPosition(Vector3f(0, 0, 0));
		_RemoteSrcLabel->SetText("Online");
		_RemoteSrcLabel->SetFontScale(0.5f);
		_RemoteSrcLabel->CalculateTextDimensions();

		UIRectf r = _LocalSrcLabel->GetPaddedRect();
		WARN("[%.2f, %.2f, %.2f, %.2f]", r.Left, r.Top, r.Right, r.Bottom);

		_RemoteSrcLabel->AlignTo(RectPosition::LEFT, _LocalSrcLabel, RectPosition::RIGHT);
		
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

}