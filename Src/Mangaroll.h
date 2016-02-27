/************************************************************************************

Filename    :   OvrApp.h
Content     :   Trivial use of the application framework.
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

************************************************************************************/

#ifndef OVRAPP_H
#define OVRAPP_H
#ifndef ANDROID
#define ANDROID
#endif

#include "App.h"
#include "SceneView.h"
#include "SoundEffectContext.h"
#include "GuiSys.h"
#include "Page.h"
#include "RemotePage.h"
#include "UI\UIMenu.h"
#include "ViewManager.h"
#include "MangaCarousel.h"
#include "Manga.h"
#include "MangaSettingsView.h"

using namespace OVR;
namespace OvrMangaroll
{

class Mangaroll : public OVR::VrAppInterface
{
public:
						Mangaroll();
	virtual				~Mangaroll();

	virtual void 		Configure( OVR::ovrSettings & settings );
	virtual void		OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI );
	virtual void		OneTimeShutdown();
	virtual bool 		OnKeyEvent( const int keyCode, const int repeatCount, const OVR::KeyEventType eventType );
	virtual OVR::Matrix4f Frame( const OVR::VrFrame & vrFrame );
	virtual OVR::Matrix4f DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms );

	class ovrLocale &	GetLocale() { return *Locale; }
	OvrGuiSys &GetGuiSys(void);
	Manga CurrentManga;
	MangaCarousel Carousel;
	MangaSettingsView MangaSettingsMenu;
private:
	ovrSoundEffectContext * SoundEffectContext;
	OvrGuiSys::SoundEffectPlayer * SoundEffectPlayer;
	OvrGuiSys *		GuiSys;
	class OVR::ovrLocale *	Locale;
	ovrMatrix4f			CenterEyeViewMatrix;
	ViewManager ViewMgr;
	double _LastPress;
};

} // namespace OvrTemplateApp

#endif	// OVRAPP_H
