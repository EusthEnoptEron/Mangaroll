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
#include <memory>
#include "GuiSys.h"
#include "Page.h"

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

	class OVR::ovrLocale &	GetLocale() { return *Locale; }

private:
	OVR::ovrSoundEffectContext * SoundEffectContext;
	OVR::OvrGuiSys::SoundEffectPlayer * SoundEffectPlayer;
	OVR::OvrGuiSys *		GuiSys;
	class OVR::ovrLocale *	Locale;
	OVR::GlProgram			Program;
	OVR::OvrSceneView		Scene;
	OVR::GlTexture			Page1;
	ovrMatrix4f			CenterEyeViewMatrix;
	OVR::GlGeometry			Quad;
	GLint				VertexTransformAttribute;
	GLuint				InstanceTransformBuffer;
	Array<Page*> Pages; 
};

} // namespace OvrTemplateApp

#endif	// OVRAPP_H
