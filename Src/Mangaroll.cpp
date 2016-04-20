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
#include "Config.h"
#include "ThreadPool.h"

#if 0
	#define GL( func )		func; EglCheckErrors();
#else
	#define GL( func )		func;
#endif

//#define MR_DEBUG

using namespace OVR;

#if defined( OVR_OS_ANDROID )
extern "C" {

jlong Java_ch_zomg_mangaroll_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	LOG( "nativeSetAppInterface" );
	OvrMangaroll::Mangaroll *mangaroll = new OvrMangaroll::Mangaroll();
	jlong num = mangaroll->SetActivity(jni, clazz, activity, fromPackageName, commandString, uriString);
	mangaroll->InitJNI(jni);
	return num;
}

} // extern "C"

#endif

namespace OvrMangaroll
{

	//==============================================================
	// ovrGuiSoundEffectPlayer
	class ovrGuiSoundEffectPlayer : public OvrGuiSys::SoundEffectPlayer
	{
	public:
		ovrGuiSoundEffectPlayer(ovrSoundEffectContext & context)
			: SoundEffectContext(context)
		{
		}

		virtual bool Has(const char * name) const OVR_OVERRIDE{ return SoundEffectContext.GetMapping().HasSound(name); }
		virtual void Play(const char * name) OVR_OVERRIDE{ SoundEffectContext.Play(name); }

	private:
		ovrSoundEffectContext & SoundEffectContext;
	};


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
	, _LastConfSync(0)
	, _Repositioning(false)
{
	CenterEyeViewMatrix = ovrMatrix4f_CreateIdentity();
}

Mangaroll::~Mangaroll()
{
	OvrGuiSys::Destroy( GuiSys );
}

void Mangaroll::InitJNI(JNIEnv *env) {
	AppState::FetcherClass = (jclass)env->NewGlobalRef(env->FindClass("ch/zomg/mangaroll/query/Fetcher"));
	AppState::ActivityClass = (jclass)env->NewGlobalRef(env->FindClass("ch/zomg/mangaroll/MainActivity"));
	AppState::ContainerFetcherClass = (jclass)env->NewGlobalRef(env->FindClass("ch/zomg/mangaroll/query/ContainerFetcher"));
	AppState::MangaFetcherClass = (jclass)env->NewGlobalRef(env->FindClass("ch/zomg/mangaroll/query/MangaFetcher"));
}
void Mangaroll::Configure( ovrSettings & settings )
{

	settings.PerformanceParms.CpuLevel = 1;
	settings.PerformanceParms.GpuLevel = 3;

	settings.EyeBufferParms.colorFormat = COLOR_8888;
	settings.EyeBufferParms.multisamples = 2;
}

void Mangaroll::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	OVR::Capture::InitForRemoteCapture();

	AppState::Instance = app;
	AppState::Reader = this;
	_Config = Config::Load(app);
	AppState::Conf = _Config;

	OVR_UNUSED( fromPackage );
	OVR_UNUSED( launchIntentJSON );
	OVR_UNUSED( launchIntentURI );
	const ovrJava * java = app->GetJava();

	SoundEffectContext = new ovrSoundEffectContext( *java->Env, java->ActivityObject );
	SoundEffectContext->Initialize();
	SoundEffectPlayer = new ovrGuiSoundEffectPlayer(*SoundEffectContext);

	Locale = ovrLocale::Create( *app, "default" );
	AppState::Scheduler = new ThreadPool(5);
	
	AppState::Strings = new TranslationManager(this);

	// Build paths
	FillStorageLocations();


	String fontName;
	GetLocale().GetString( "@string/font_name", "efigs.fnt", fontName );
	GuiSys->Init( this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines() );
	GuiSys->GetDefaultFont().Load("", "assets/mangaroll.fnt");

	Carousel.OneTimeInit(launchIntentURI);
	MangaSettingsMenu.OneTimeInit(launchIntentURI);
	MangaSelectionMenu.OneTimeInit(launchIntentURI);

	ViewMgr.AddView(&MangaSettingsMenu);
	ViewMgr.AddView(&MangaSelectionMenu);

	Time::Delta = 0;
	Time::Elapsed = vrapi_GetTimeInSeconds();

	ViewMgr.OpenView(MangaSelectionMenu);

}

void Mangaroll::FillStorageLocations() {
	JNIEnv* env = app->GetJava()->Env;
	jmethodID getDirsMethod = env->GetMethodID(AppState::ActivityClass, "getAllStorageDirectories", "()[Ljava/lang/String;");
	jobjectArray dirList = (jobjectArray)env->CallObjectMethod(app->GetJava()->ActivityObject, getDirsMethod);
	if (dirList != NULL) {
		int count = env->GetArrayLength(dirList);
		for (int i = 0; i < count; i++) {
			String path = JavaUTFChars(env, (jstring)env->GetObjectArrayElement(dirList, i)).ToStr();
			
			if (HasPermission(path.ToCStr(), PERMISSION_READ)) {
				_MangaPaths.PushBack(path + "RetailMedia/Manga/");
				_MangaPaths.PushBack(path + "Manga/");
				_MangaPaths.PushBack(path + "RetailMedia/Comics/");
				_MangaPaths.PushBack(path + "Comics/");
			}
		}
	}
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
	_Config->Save();

	delete _Config;

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
	if (keyCode == OVR_KEY_ESCAPE && eventType == KeyEventType::KEY_EVENT_LONG_PRESS) {
		// Crash
		AppState::Conf->Orientation = Quatf();
	}

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
	Quatf orientation = (Quatf)vrFrame.Tracking.HeadPose.Pose.Orientation;
	Vector3f v0 = Vector3f(0, 0, -1.0f);
	Vector3f v1 = Vector3f(0, 1.0f, 0);
	
	if (_Repositioning) {
		AppState::Conf->Orientation = orientation * _OrientationOffset;
	}

	Vector3f lookAt = ((Quatf)vrFrame.Tracking.HeadPose.Pose.Orientation) * v0;

	// UPDATE GLOBAL HELPERS
	Time::Delta = vrFrame.DeltaSeconds;
	Time::Elapsed = vrapi_GetTimeInSeconds();
	Frame::Current = &vrFrame;
	HMD::Direction = lookAt;
	HMD::NormalizedDirection = AppState::Conf->Orientation.Inverted() * lookAt;

	if (Time::Elapsed - _LastConfSync > 60) {
		_Config->Save();
		_LastConfSync = Time::Elapsed;
	}

	
	AsyncTextureManager::Instance().Update();
	Animator.Progress(Time::Delta);

	if (_Repositioning && vrFrame.Input.buttonReleased & BUTTON_TOUCH) {
		_Repositioning = false;
	}
	else if (!_Repositioning && vrFrame.Input.buttonPressed & BUTTON_TOUCH_LONGPRESS && !LongPressInterrupted) {
		_Repositioning = true;

		Vector3f currentUp = orientation * v1;
		Vector3f offsetForward = AppState::Conf->Orientation * v0;
		
		Quatf startOrientation = Quatf(Matrix4f::LookAtRH(Vector3f::ZERO, offsetForward, currentUp)).Inverted();
		_OrientationOffset = (startOrientation.Inverted() * orientation).Inverted();

	}

	if (vrFrame.Input.buttonReleased & BUTTON_TOUCH) {
		LongPressInterrupted = false;
	}


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
			GetGuiSys().GetGazeCursor().ClearGhosts();

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
