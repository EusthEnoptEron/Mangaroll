/************************************************************************************

Filename    :   OvrApp.cpp
Content     :   Trivial use of the application framework.
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/

#include "Mangaroll.h"
#include "GuiSys.h"
#include "OVR_Locale.h"
#include "Kernel/OVR_Hash.h"
#include "Kernel/OVR_Types.h"
#include "Helpers.h"
#include <OVR_Capture.h>
#include "AsyncTexture.h"

#if 0
	#define GL( func )		func; EglCheckErrors();
#else
	#define GL( func )		func;
#endif

//#define MR_DEBUG

using namespace OVR;

#if defined( OVR_OS_ANDROID )
extern "C" {

jlong Java_oculus_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	LOG( "nativeSetAppInterface" );
	return (new OvrMangaroll::Mangaroll())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}

} // extern "C"

#endif

namespace OvrMangaroll
{

Mangaroll::Mangaroll()
	: GuiSys( OvrGuiSys::Create() )
	, CurrentManga(NULL)
	, Carousel(this)
	, MangaSettingsMenu(this)
	, MangaSelectionMenu(*this)
	, Animator()
	, SoundEffectContext( NULL )
	, SoundEffectPlayer( NULL )
	, Locale( NULL )
	, ViewMgr()
	, _MenuOpen(false)
{
	CenterEyeViewMatrix = ovrMatrix4f_CreateIdentity();
}

Mangaroll::~Mangaroll()
{
	OvrGuiSys::Destroy( GuiSys );
}

void Mangaroll::Configure( ovrSettings & settings )
{
	settings.PerformanceParms.CpuLevel = 0;
	settings.PerformanceParms.GpuLevel = 2;

	settings.EyeBufferParms.colorFormat = COLOR_8888;
	settings.EyeBufferParms.multisamples = 2;
}

void Mangaroll::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	OVR::Capture::InitForRemoteCapture();
	AppState::Instance = app;

	OVR_UNUSED( fromPackage );
	OVR_UNUSED( launchIntentJSON );
	OVR_UNUSED( launchIntentURI );
	const ovrJava * java = app->GetJava();

	SoundEffectContext = new ovrSoundEffectContext( *java->Env, java->ActivityObject );
	SoundEffectContext->Initialize();
	SoundEffectPlayer = new OvrGuiSys::ovrDummySoundEffectPlayer();

	Locale = ovrLocale::Create( *app, "default" );
	
	String fontName;
	GetLocale().GetString( "@string/font_name", "efigs.fnt", fontName );
	GuiSys->Init( this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines() );

	Carousel.OneTimeInit(launchIntentURI);
	MangaSettingsMenu.OneTimeInit(launchIntentURI);
	MangaSelectionMenu.OneTimeInit(launchIntentURI);

	ViewMgr.AddView(&MangaSettingsMenu);
	ViewMgr.AddView(&MangaSelectionMenu);

	Time::Delta = 0;
	Time::Elapsed = vrapi_GetTimeInSeconds();

	ViewMgr.OpenView(MangaSelectionMenu);

}

void Mangaroll::SelectManga() {
	ViewMgr.OpenView(MangaSelectionMenu);
}

void Mangaroll::ShowManga(Manga *manga) {
	if (manga != NULL) {
		CurrentManga = manga;
		Carousel.SetManga(manga);
	}

	ViewMgr.OpenView(MangaSettingsMenu);
	
}

void Mangaroll::OneTimeShutdown()
{
	OVR::Capture::Shutdown();
	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	Carousel.OneTimeShutdown();
	MangaSettingsMenu.OneTimeShutdown();
}

bool Mangaroll::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
#ifdef MR_DEBUG
	// Crash button
	if (keyCode == OVR_KEY_ESCAPE && eventType == KeyEventType::KEY_EVENT_LONG_PRESS) {
		// Crash
		((Mangaroll *)NULL)->OnKeyEvent(keyCode, repeatCount, eventType);
	}
#endif

	if ( GuiSys->OnKeyEvent( keyCode, repeatCount, eventType ) )
	{
		return true;
	}
	return ViewMgr.OnKeyEvent(keyCode, repeatCount, eventType);
}

OvrGuiSys &Mangaroll::GetGuiSys(void) {
	return *GuiSys;
}


Matrix4f Mangaroll::Frame( const VrFrame & vrFrame )
{
	Vector3f v0 = Vector3f(0, 0, -1.0f);
	Vector3f lookAt = ((Quatf)vrFrame.Tracking.HeadPose.Pose.Orientation) * v0;

	// UPDATE GLOBAL HELPERS
	Time::Delta = vrFrame.DeltaSeconds;
	Time::Elapsed = vrapi_GetTimeInSeconds();
	Frame::Current = &vrFrame;
	HMD::Direction = lookAt;

	
	AsyncTextureManager::Instance().Update();
	Animator.Progress(Time::Delta);

	// FRAME STEPS
	CenterEyeViewMatrix = ViewMgr.Frame(vrFrame);
	GuiSys->Frame( vrFrame, CenterEyeViewMatrix );

	if (GetGuiSys().IsAnyMenuOpen() != _MenuOpen) {
		_MenuOpen = !_MenuOpen;

		if (_MenuOpen) {
			GetGuiSys().GetGazeCursor().ShowCursor();
			//GetGuiSys().GetGazeCursor().ForceDistance(0.5f, eGazeCursorStateType::CURSOR_STATE_NORMAL);
			Carousel.MoveOut();
		}
		else {
			GetGuiSys().GetGazeCursor().HideCursor();
			Carousel.MoveIn();
		}
	}

	return CenterEyeViewMatrix;
}



Matrix4f Mangaroll::DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms )
{
	const Matrix4f eyeViewProjection = ViewMgr.DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
	
	GuiSys->RenderEyeView(CenterEyeViewMatrix, ViewMgr.GetEyeViewMatrix(eye), ViewMgr.GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY));

	return eyeViewProjection;
}

} // namespace OvrTemplateApp
