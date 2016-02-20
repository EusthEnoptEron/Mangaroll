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

	static const char VERTEX_SHADER[] =
#if defined( OVR_OS_ANDROID )
	"#version 300 es\n"
#endif
	"in vec3 Position;\n"
	"in vec4 VertexColor;\n"
	"in vec2 TexCoord;\n"
	"uniform mat4 Viewm;\n"
	"uniform mat4 Projectionm;\n"
	"uniform mat4 Modelm;\n"
	"out vec4 fragmentColor;\n"
	"out vec2 oTexCoord;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = Projectionm * Viewm * Modelm * vec4( Position, 1.0 );\n"
	"	fragmentColor = VertexColor;\n"
	"   oTexCoord = TexCoord;\n"
	"}\n";

static const char FRAGMENT_SHADER[] =
#if defined( OVR_OS_ANDROID )
	"#version 300 es\n"
#endif
	"in lowp vec4 fragmentColor;\n"
	"uniform sampler2D Texture0;\n"
	"uniform int IsSelected;\n"
	"in lowp vec2 oTexCoord;\n"
	"out lowp vec4 outColor;\n"
	"void main()\n"
	"{\n"
	"	outColor = IsSelected == 1 ? fragmentColor : texture( Texture0, oTexCoord );\n"
	"	//outColor = fragmentColor;\n"
	"}\n";

// setup Cube
struct ovrQuadVertices
{
	Vector3f positions[4];
	Vector4f colors[4];
	Vector2f uv0[4];
};

static ovrQuadVertices quadVertices =
{
	// positions
	{
		Vector3f( +1.0f, +1.0f, -5.0f ), Vector3f( -1.0f, +1.0f, -5.0f ), Vector3f( -1.0f, -1.0f, -5.0f ), Vector3f( +1.0f, -1.0f, -5.0f ),
	},
	// colors
	{
		Vector4f( 1.0f, 0.0f, 1.0f, 1.0f ), Vector4f( 0.0f, 1.0f, 0.0f, 1.0f ), Vector4f( 0.0f, 0.0f, 1.0f, 1.0f ), Vector4f( 1.0f, 0.0f, 0.0f, 1.0f )
	},
	// uvs
	{
		Vector2f(1, 1), Vector2f(0, 1), Vector2f(0, 0), Vector2f(1, 0)
	}
};


static const unsigned short quadIndices[12] =
{
	0, 1, 2, 2, 3, 0,	// front
	0, 2, 1, 2, 0, 3,	// back
};


Mangaroll::Mangaroll()
	: SoundEffectContext( NULL )
	, SoundEffectPlayer( NULL )
	, GuiSys( OvrGuiSys::Create() )
	, Locale( NULL )
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
	WARN("YAY");
	const ovrJava * java = app->GetJava();
	
	SoundEffectContext = new ovrSoundEffectContext( *java->Env, java->ActivityObject );
	SoundEffectContext->Initialize();
	SoundEffectPlayer = new OvrGuiSys::ovrDummySoundEffectPlayer();

	Locale = ovrLocale::Create( *app, "default" );
	
	String fontName;
	GetLocale().GetString( "@string/font_name", "efigs.fnt", fontName );
	GuiSys->Init( this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines() );

	int Width, Height;
	WARN("WHOO %s", "Lol");
	Page1 = OVR::LoadTextureFromApplicationPackage("assets/page0006.jpg", TextureFlags_t( TEXTUREFLAG_NO_DEFAULT ), Width, Height);
	WARN("WIIIDITH %d",  Width);
	// Create the program.
	Program = BuildProgram( VERTEX_SHADER, FRAGMENT_SHADER );

	// Create the quad.
	VertexAttribs attribs;
	attribs.position.Resize( 4 );
	attribs.color.Resize( 4 );
	attribs.uv0.Resize(4);
	for ( int i = 0; i < 4; i++ )
	{
		attribs.position[i] = quadVertices.positions[i];
		attribs.color[i] = quadVertices.colors[i];
		attribs.uv0[i] = quadVertices.uv0[i];
	}

	Array< TriangleIndex > indices;
	indices.Resize( 12 );
	for ( int i = 0; i < 12; i++ )
	{
		indices[i] = quadIndices[i];
	}

	Quad.Create( attribs, indices );

	

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
	Pages.Resize(images.GetSizeI());

	int y = 0;
	for(int i = 0; i < images.GetSizeI(); i++) {
		WARN("%s -> %s", images[i].ToCStr(), images[i].GetExtension().ToCStr());
		if(images[i].GetExtension() == ".jpg") {
			WARN("YEAH");
			Pages[y++] = new LocalPage(images[i]);
		}
	}
	Pages.Resize(y);
	if(y > 0) {
		Pages[0]->SetOffset(0);

		if(y > 1) {
			for(int i = 0; i < Pages.GetSizeI() - 1; i++) {
				Pages[i]->SetNext(Pages[i+1]);
			}
		}
	}

/*
	Pages.Resize(11);
	Pages[0] = new Page("assets/page0006.jpg");
	Pages[1] = new Page("assets/page0007.jpg");
	Pages[2] = new Page("assets/page0008.jpg");
	Pages[3] = new Page("assets/page0009.jpg");
	Pages[4] = new Page("assets/page0010.jpg");
	Pages[5] = new Page("assets/page0011.jpg");
	Pages[6] = new Page("assets/page0012.jpg");
	Pages[7] = new Page("assets/page0013.jpg");
	Pages[8] = new Page("assets/page0014.jpg");
	Pages[9] = new Page("assets/page0015.jpg");
	Pages[10] = new Page("assets/page0016.jpg");

	Pages[0]->SetNext(Pages[1]);
	Pages[1]->SetNext(Pages[2]);
	Pages[2]->SetNext(Pages[3]);
	Pages[3]->SetNext(Pages[4]);
	Pages[4]->SetNext(Pages[5]);
	Pages[5]->SetNext(Pages[6]);
	Pages[6]->SetNext(Pages[7]);
	Pages[7]->SetNext(Pages[8]);
	Pages[8]->SetNext(Pages[9]);
	Pages[9]->SetNext(Pages[10]);
	
	Pages[0]->SetOffset(0);*/


}


void Mangaroll::OneTimeShutdown()
{
	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	DeleteProgram( Program );

	for(int i = 0; i < Pages.GetSizeI(); i++) {
		delete Pages[i];
	}

}

bool Mangaroll::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
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
	const Matrix4f m = CenterEyeViewMatrix;

	Vector3f v0 = Vector3f(0, 0, -1.0f);
	Vector3f lookAt = m.Transform(v0).Normalized();

	float angleA = RadToDegree(atan2(prevLookAt[0], prevLookAt[2]));
    float angleB = RadToDegree(atan2(lookAt[0], lookAt[2]));
    float angleDiff = deltaAngle(angleA, angleB);
    //angleDiff *= -1;
	angle += angleDiff;

	prevLookAt = lookAt;

	//WARN("Angle: %f", angle);
	// Update GUI systems after the app frame, but before rendering anything.
	GuiSys->Frame( vrFrame, CenterEyeViewMatrix );

	for(int i = 0; i < Pages.GetSizeI(); i++) {
		Pages[i]->Update(angle);
	}

	return CenterEyeViewMatrix;
}



Matrix4f Mangaroll::DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms )
{
	const Matrix4f eyeViewMatrix = vrapi_GetEyeViewMatrix( &app->GetHeadModelParms(), &CenterEyeViewMatrix, eye );
	const Matrix4f eyeProjectionMatrix = ovrMatrix4f_CreateProjectionFov( fovDegreesX, fovDegreesY, 0.0f, 0.0f, 0.1f, 0.0f );
	const Matrix4f eyeViewProjection = eyeProjectionMatrix * eyeViewMatrix;

	GL( glClearColor( .5f, .5f, .5f, 1.0f ) );
	GL( glClear( GL_COLOR_BUFFER_BIT ) );
	GL( glUseProgram( Program.program ) );
	GL( glUniformMatrix4fv( Program.uView, 1, GL_TRUE, eyeViewMatrix.M[0] ) );
	GL( glUniformMatrix4fv( Program.uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0] ) );

	for(int i = 0; i < Pages.GetSizeI(); i++) {
		Pages[i]->Draw(Program);
	}
/*
	GL( glActiveTexture(GL_TEXTURE0) );
	GL( glBindTexture(Page1.target, Page1.texture) );
	GL( glBindVertexArray( Quad.vertexArrayObject ) );
	GL( glDrawElements( GL_TRIANGLES, Quad.indexCount, GL_UNSIGNED_SHORT, NULL ) );*/
	GL( glBindVertexArray( 0 ) );
	GL( glUseProgram( 0 ) );


	frameParms.ExternalVelocity = Scene.GetExternalVelocity();
	frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

	GuiSys->RenderEyeView( Scene.GetCenterEyeViewMatrix(), eyeViewMatrix, eyeProjectionMatrix );

	return eyeViewProjection;
}

} // namespace OvrTemplateApp
