#pragma once

#include "View.h"
#include "UI\UILabel.h"
#include "UI\UISlider.h"
#include "UI\UIProgressBar.h"

using namespace OVR;
namespace OvrMangaroll {
	
	class Mangaroll;

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


	private:
		Mangaroll *_Manga;

		UILabel *_TitleLabel;
		UILabel *_PageLabel;
		UIProgressBar *_ProgressBar;

		UISlider *_GammaSlider;
		UISlider *_ContrastSlider;
		UISlider *_BrightnessSlider;

	};
}