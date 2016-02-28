#pragma once

#include "View.h"
#include "UI\UILabel.h"
#include "UI\UISlider.h"
#include "UI\UIDiscreteSlider.h"
#include "UI\UIProgressBar.h"
#include "UI\UIContainer.h"

using namespace OVR;
namespace OvrMangaroll {
	
	class Mangaroll;

	class ScrubBarComponent : public VRMenuComponent
	{
	public:
		static const int 		TYPE_ID = 152414;

		ScrubBarComponent();

		virtual int				GetTypeId() const { return TYPE_ID; }

		void					SetMax(const int duration);
		int						GetMax(void);
		void					SetOnClick(void(*callback)(ScrubBarComponent *, void *, float), void *object);
		void					SetOnText(void(*callback)(ScrubBarComponent *, void*, UILabel *label, int progress), void *object);
		void					SetWidgets(UIMenu *menu, UIObject *background, UIObject *scrubBar, UILabel *currentTime, UILabel *seekTime, const int scrubBarWidth, const int scrubBarHeight);
		void 					SetProgress(const float progress);

		bool					IsScrubbing() const { return TouchDown; }

	private:
		bool					HasFocus;
		bool					TouchDown;

		float					Progress;
		int						Max;

		UIMenu *				Menu;
		UIObject *				Background;
		UIObject *				ScrubBar;
		UILabel *				CurrentTime;
		UILabel *				SeekTime;
		int 					ScrubBarWidth;
		int						ScrubBarHeight;

		void(*OnClickFunction)(ScrubBarComponent *button, void *object, float progress);
		void(*OnSetTextFunction)(ScrubBarComponent *button, void *object, UILabel *label, int progress);
		void *					OnClickObject;
		void *					OnTextObject;

	private:
		virtual eMsgStatus      OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);

		eMsgStatus 				OnFrame(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);

		void 					OnClick(OvrGuiSys & guiSys, VrFrame const & vrFrame, VRMenuEvent const & event);

		//void 					SetTimeText(UILabel *label, const int time);
	};

	class MangaSettingsView : public View
	{
	public:
		MangaSettingsView(Mangaroll *app);
		~MangaSettingsView();


		bool 		OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType) ;
		Matrix4f 	Frame(const VrFrame & vrFrame) ;
		Matrix4f	GetEyeViewMatrix(const int eye) const ;
		Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const ;
		Matrix4f 	DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) ;

		void 		OneTimeInit(const char * launchIntent) ;
		void		OneTimeShutdown() ;

		void 		OnOpen() ;
		void 		OnClose() ;
		void SetPageProgress(float);

	private:
		void ShowGUI(void);
		void HideGUI(void);
		void UpdateMenuState(void);

		Mangaroll *_Mangaroll;

		UIMenu *_Menu;
		UIContainer *_CenterContainer;
		UIContainer *_LeftContainer;
		UIContainer *_RightContainer;

		UILabel *_TitleLabel;
		UILabel *_PageLabel;

		UIDiscreteSlider *_GammaSlider;
		UISlider *_ContrastSlider;
		UISlider *_BrightnessSlider;

		SineFader _Fader;
		ScrubBarComponent _ProgressComponent;
		
		UILabel *_PageSeekLabel;

		UIImage *_ProgressBG;
		UIImage *_ProgressFG;

		UITexture _ProgressBGTexture;
		UITexture _ProgressFGTexture;

	};
}