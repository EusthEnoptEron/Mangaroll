#pragma once

#include "View.h"
#include "UI\UILabel.h"
#include "UI\UISlider.h"
#include "UI\UIDiscreteSlider.h"
#include "UI\UIProgressBar.h"
#include "UI\UIContainer.h"
#include "UICheckbox.h"

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
		float					GetProgress(void);
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

	class UIScrubBar : public UIObject {
	public:
		UIScrubBar(OvrGuiSys &guiSys, int width, int height);
		~UIScrubBar();

		void AddToMenu(UIMenu *menu, UIObject *parent = NULL);
		ScrubBarComponent &GetComponent(void);
		void SetIndicators(UILabel *, UILabel *);
		void SetFillColor(Vector4f);
	private:
		int _Width;
		int _Height;
		ScrubBarComponent _ProgressComponent;
		void Init(void);

		UIImage *_BGImage;
		UIImage *_FGImage;
	};

	class MangaSettingsView : public View
	{
	public:
		MangaSettingsView(Mangaroll *app);
		virtual ~MangaSettingsView();


		virtual bool 		OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType);
		virtual Matrix4f 	Frame(const VrFrame & vrFrame);
		virtual Matrix4f	GetEyeViewMatrix(const int eye) const;
		virtual Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const;
		virtual Matrix4f 	DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms);

		virtual void 		OneTimeInit(const char * launchIntent);
		virtual void		OneTimeShutdown();

		virtual void 		OnOpen();
		virtual void 		OnClose();
		void SetPageProgress(float);
		void ShowGUI(void);
		void HideGUI(void);
	private:
		static void OnShaderChanged(UICheckbox *, void *, bool);
		static void OnReadDirChanged(UICheckbox *, void *, bool);
		static void OnAutoRotateChanged(UICheckbox *, void *, bool);
		static void OnGuideChanged(UICheckbox *, void *, bool);
		void UpdateMenuState(void);

		Mangaroll *_Mangaroll;
		bool _CloseRequested;

		UIMenu *_Menu;
		UIContainer *_OrientationContainer;
		UIContainer *_CenterContainer;
		UIContainer *_LeftContainer;
		UIContainer *_RightContainer;

		UILabel *_TitleLabel;
		UILabel *_PageLabel;

		UIDiscreteSlider *_GammaSlider;
		UIScrubBar *_ContrastSlider;
		UILabel *_ContrastLabel;
		UIScrubBar *_BrightnessSlider;
		UILabel *_BrightnessLabel;
		UIImage *_FXBG;
		UIImage *_OptionsBG;
		UIImage *_MainBG;

		SineFader _Fader;
		ScrubBarComponent _ProgressComponent;
		
		UILabel *_PageSeekLabel;

		UIImage *_ProgressBG;
		UIImage *_ProgressFG;
		UIImage *_SeekPos;

		UITexture _ProgressBGTexture;
		UITexture _ProgressFGTexture;
		UITexture _CloseTexture;
		UITexture _FXBGTexture;
		UITexture _MainBGTexture;
		UITexture _OptionsBGTexture;
		UITexture _SeekPosTexture;
		

		float _MainContainerWidth;

		UIButton *_CloseButton;
		UICheckbox *_ShaderToggle;
		UICheckbox *_AutoToggle;
		UICheckbox *_ReadDirToggle;
		UICheckbox *_HelperToggle;
	};
}