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


#if 0
	#define GL( func )		func; EglCheckErrors();
#else
	#define GL( func )		func;
#endif

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
	: CurrentManga()
	, Carousel(this)
	, MangaSettingsMenu(this)
	, SoundEffectContext( NULL )
	, SoundEffectPlayer( NULL )
	, GuiSys( OvrGuiSys::Create() )
	, Locale( NULL )
	, ViewMgr()
{
	CenterEyeViewMatrix = ovrMatrix4f_CreateIdentity();
}

Mangaroll::~Mangaroll()
{
	OvrGuiSys::Destroy( GuiSys );
}

void Mangaroll::Configure( ovrSettings & settings )
{
	settings.PerformanceParms.CpuLevel = 2;
	settings.PerformanceParms.GpuLevel = 2;
}

void Mangaroll::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
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

	ViewMgr.AddView(&MangaSettingsMenu);

	// --- LOAD MANGA PAGES ---
	const OvrStoragePaths & paths = app->GetStoragePaths();

	Array<String> SearchPaths;
	paths.PushBackSearchPathIfValid( EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths );
	paths.PushBackSearchPathIfValid( EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths );
	paths.PushBackSearchPathIfValid( EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths );
	paths.PushBackSearchPathIfValid( EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths );

	StringHash<String> results = RelativeDirectoryFileList(SearchPaths, "Manga/");
	String mangaPath;

	for(StringHash<String>::Iterator it = results.Begin(); it != results.End(); ++it) {
		if(it->Second.GetCharAt(it->Second.GetLengthI()-1) == '/') {
			mangaPath = GetFullPath(SearchPaths, it->Second);
			break;
		}
	}

	Array<String> images = DirectoryFileList(mangaPath.ToCStr());
	
	CurrentManga.Name = ExtractDirectory(mangaPath);
	for(int i = 0; i < images.GetSizeI(); i++) {
		//WARN("%s -> %s", images[i].ToCStr(), images[i].GetExtension().ToCStr());
		if(images[i].GetExtension() == ".jpg") {
			CurrentManga.AddPage(new LocalPage(images[i]));
		}
	}

	Carousel.SetManga(&CurrentManga);

	//GuiSys->GetGazeCursor().ShowCursor();

	Time::Delta = 0;
	Time::Elapsed = vrapi_GetTimeInSeconds();

	ViewMgr.OpenView(MangaSettingsMenu);

}


void Mangaroll::OneTimeShutdown()
{
	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	Carousel.OneTimeShutdown();
	MangaSettingsMenu.OneTimeShutdown();
}

bool Mangaroll::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
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

	// FRAME STEPS
	CenterEyeViewMatrix = ViewMgr.Frame(vrFrame);
	GuiSys->Frame( vrFrame, CenterEyeViewMatrix );

	return CenterEyeViewMatrix;
}



Matrix4f Mangaroll::DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms )
{
	const Matrix4f eyeViewProjection = ViewMgr.DrawEyeView(eye, fovDegreesX, fovDegreesY, frameParms);
	
	GuiSys->RenderEyeView(CenterEyeViewMatrix, ViewMgr.GetEyeViewMatrix(eye), ViewMgr.GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY));

	return eyeViewProjection;
}

} // namespace OvrTemplateApp
