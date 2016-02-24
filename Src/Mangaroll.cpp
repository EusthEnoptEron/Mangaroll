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
	: SoundEffectContext( NULL )
	, SoundEffectPlayer( NULL )
	, GuiSys( OvrGuiSys::Create() )
	, Locale( NULL )
	, Carousel(NULL)
	, Metadata(NULL)
	, Browser(NULL)
	, LastPress(0)
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
	Carousel = new PageCarousel();

	for(int i = 0; i < images.GetSizeI(); i++) {
		//WARN("%s -> %s", images[i].ToCStr(), images[i].GetExtension().ToCStr());
		if(images[i].GetExtension() == ".jpg") {
			//Carousel->AddPage(new LocalPage(images[i]));
		}
	}

	//// MAKE MENU
	OvrMetaDataFileExtensions fileExtensions;
	fileExtensions.GoodExtensions.PushBack(".jpg");

	Metadata = new MangaMetadata();
	Metadata->InitFromDirectory("Manga/", SearchPaths, fileExtensions);
	Browser = (MangaBrowser *)MangaBrowser::Create(
		*this,
		*GuiSys,
		*Metadata,
		256, 20.0f,
		256, 200.0f,
		7,
		5.4f
	);
	GuiSys->AddMenu(Browser);
	
	
	Browser->SetFlags(VRMenuFlags_t(VRMENU_FLAG_PLACE_ON_HORIZON));
	Browser->SetFolderTitleSpacingScale(0.37f);
	Browser->SetPanelTextSpacingScale(0.34f);
	Browser->SetScrollBarSpacingScale(0.9f);
	Browser->SetScrollBarRadiusScale(1.0f);

	//OvrMetaData *metadata = new OvrMetaData(); = MetaData->InitFromDirectory( videosDirectory, SearchPaths, fileExtensions );

	Browser->OneTimeInit(*GuiSys);
	Browser->BuildDirtyMenu(*GuiSys, *Metadata);

	GuiSys->OpenMenu(OvrFolderBrowser::MENU_NAME);
	GuiSys->GetGazeCursor().ShowCursor();

	Time::Delta = 0;
	Time::Elapsed = vrapi_GetTimeInSeconds();
}


void Mangaroll::OneTimeShutdown()
{
	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	delete Carousel;

}

bool Mangaroll::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
	WARN("KEY: %d", keyCode);
	if ( GuiSys->OnKeyEvent( keyCode, repeatCount, eventType ) )
	{
		return true;
	}
	return false;
}


Vector3f prevLookAt = Vector3f(0, 0, -1.0f);
float angle = 0;

float deltaAngle(float angle1, float angle2) {
    float a = angle1 - angle2;
    a += (a>180) ? -360 : (a<-180) ? 360 : 0;

    return  a;
}
Matrix4f Mangaroll::Frame( const VrFrame & vrFrame )
{
	CenterEyeViewMatrix = vrapi_GetCenterEyeViewMatrix( &app->GetHeadModelParms(), &vrFrame.Tracking, NULL );

	Vector3f v0 = Vector3f(0, 0, -1.0f);
	Vector3f lookAt = ((Quatf)vrFrame.Tracking.HeadPose.Pose.Orientation) * v0;

	float angleA = RadToDegree(atan2(prevLookAt[0], prevLookAt[2]));
	float angleB = RadToDegree(atan2(lookAt[0], lookAt[2]));
	float angleDiff = deltaAngle(angleA, angleB);
	//angleDiff *= -1;
	angle -= angleDiff;

	prevLookAt = lookAt;

	// UPDATE GLOBAL HELPERS
	Time::Delta = vrFrame.DeltaSeconds;
	Time::Elapsed = vrapi_GetTimeInSeconds();
	Frame::Current = &vrFrame;
	HMD::Direction = lookAt;

	if(vrFrame.Input.buttonReleased && Time::Elapsed - LastPress > 0.5f ) {
		AppState::Guide = (GuideType)((AppState::Guide + 1) % 3);
		LastPress = Time::Elapsed;
	}
	// ---------------------

	//WARN("Angle: %f", angle);
	// Update GUI systems after the app frame, but before rendering anything.
	GuiSys->Frame( vrFrame, CenterEyeViewMatrix );

	Carousel->Update(angle);

	return CenterEyeViewMatrix;
}



Matrix4f Mangaroll::DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms )
{
	const Matrix4f eyeViewMatrix = vrapi_GetEyeViewMatrix( &app->GetHeadModelParms(), &CenterEyeViewMatrix, eye );
	const Matrix4f eyeProjectionMatrix = ovrMatrix4f_CreateProjectionFov( fovDegreesX, fovDegreesY, 0.0f, 0.0f, 0.1f, 0.0f );
	const Matrix4f eyeViewProjection = eyeProjectionMatrix * eyeViewMatrix;

	GL( glClearColor( .5f, .5f, .5f, 1.0f ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );

	Carousel->Render(Matrix4f::Identity(), CenterEyeViewMatrix, eyeViewMatrix, eyeProjectionMatrix);

	GL( glBindVertexArray( 0 ) );
	GL( glUseProgram( 0 ) );


	frameParms.ExternalVelocity = Scene.GetExternalVelocity();
	frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

	GuiSys->RenderEyeView( CenterEyeViewMatrix, eyeViewMatrix, eyeProjectionMatrix );

	return eyeViewProjection;
}

} // namespace OvrTemplateApp
