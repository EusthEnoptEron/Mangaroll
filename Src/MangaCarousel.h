#pragma once

#include "Kernel\OVR_Math.h"
#include "GlProgram.h"
#include "Input.h"
#include "SceneView.h"
#include "Manga.h"
#include "Interpolation.h"
#include "Fader.h"
#include "IScene.h"

using namespace OVR;
namespace OvrMangaroll {
	class Mangaroll;

	class MangaCarousel
	{
	public:
		MangaCarousel(Mangaroll *mangaroll);
		~MangaCarousel();

		void OneTimeInit(const char * launchIntent);
		void OneTimeShutdown();
		Matrix4f Frame(const VrFrame & vrFrame);

		Matrix4f GetEyeViewMatrix(const int eye) const;
		Matrix4f GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const;
		Matrix4f DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms);
		void SetManga(Manga *manga);
		OvrSceneView Scene;
		Manga *CurrentManga;
		Manga *NextManga;
		void MoveOut(void);
		void MoveIn(void);
		float GetAngle() {
			return _Angle;
		}
		void ChangeDirection();

	private:
		GlProgram *_Prog;
		GlProgram *_Progs[2];
		int _uContrast[2];
		int _uBrightness[2];
		

		ovrMatrix4f _CenterEyeViewMatrix;
		Mangaroll *_Mangaroll;
		Vector3f _PrevLookAt;
		float _Angle;
		float _LastPress;
		SineFader _Fader;
		bool _Operatable;
		bool _Scaling;
		// Zoom at start
		float _StartZoom;
		// The current forward angle
		float _ForwardAngle;
		// Angle that has to be added to the current angle, only processed bit by bit
		Tween _AngleAnimator;
		//ViewManager _SceneManager;
		Array<IScene*> _Scenes;
		IScene *_CurrentScene;
		// Deg
		static const float SCROLL_ANGLE_MIN;
		static const float SCROLL_ANGLE_MAX;

		// Deg / sec
		static const float SCROLL_SPEED_MIN;
		static const float SCROLL_SPEED_MAX;


	};



}