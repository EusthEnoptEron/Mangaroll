#include "MangaCarousel.h"
#include "Helpers.h"
#include "ShaderManager.h"
#include "Mangaroll.h"

#include "GLES3\gl3_loader.h"



namespace OvrMangaroll {
	float deltaAngle(float angle1, float angle2) {
		float a = angle1 - angle2;
		a += (a>180) ? -360 : (a<-180) ? 360 : 0;

		return  a;
	}

	MangaCarousel::MangaCarousel(Mangaroll *app) 
		: Scene()
		, CurrentManga(NULL)
		, NextManga(NULL)
		, _Prog(NULL)
		, _CenterEyeViewMatrix()
		, _Mangaroll (app)
		, _PrevLookAt(0, 0, -1.0f)
		, _Angle(0)
		, _LastPress(0)
	{
	}

	MangaCarousel::~MangaCarousel()
	{
	}

	void MangaCarousel::OneTimeInit(const char * launchIntent) {
		_Prog = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
	}
	void MangaCarousel::OneTimeShutdown(){

	}

	Matrix4f MangaCarousel::Frame(const VrFrame & vrFrame){
		_CenterEyeViewMatrix = vrapi_GetCenterEyeViewMatrix(&_Mangaroll->app->GetHeadModelParms(), &vrFrame.Tracking, NULL);

		float angleA = RadToDegree(atan2(_PrevLookAt[0], _PrevLookAt[2]));
		float angleB = RadToDegree(atan2(HMD::Direction[0], HMD::Direction[2]));
		float angleDiff = deltaAngle(angleA, angleB);
		//angleDiff *= -1;
		_Angle -= angleDiff;

		_PrevLookAt = HMD::Direction;

		//WARN("%.2f", _Angle);
		if (vrFrame.Input.buttonReleased && Time::Elapsed - _LastPress > 0.5f) {
			AppState::Guide = (GuideType)((AppState::Guide + 1) % 3);
			_LastPress = Time::Elapsed;
		}

		if (CurrentManga != NULL) {
			CurrentManga->Update(_Angle);
		}

		Scene.Frame(vrFrame, _Mangaroll->app->GetHeadModelParms());

		return _CenterEyeViewMatrix;
	}

	Matrix4f MangaCarousel::GetEyeViewMatrix(const int eye) const {
		return vrapi_GetEyeViewMatrix(&_Mangaroll->app->GetHeadModelParms(), &_CenterEyeViewMatrix, eye);
	}

	Matrix4f MangaCarousel::GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const {
		return ovrMatrix4f_CreateProjectionFov(fovDegreesX, fovDegreesY, 0.0f, 0.0f, 0.1f, 0.0f);
	}

	Matrix4f MangaCarousel::DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms) {
		const Matrix4f eyeViewMatrix = GetEyeViewMatrix(eye);
		const Matrix4f eyeProjectionMatrix = GetEyeProjectionMatrix(eye, fovDegreesX, fovDegreesY);
		const Matrix4f eyeViewProjection = eyeProjectionMatrix * eyeViewMatrix;

		glClearColor(.5f, .5f, .5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (CurrentManga != NULL) {
			glUseProgram(_Prog->program);
			glUniformMatrix4fv(_Prog->uView, 1, GL_TRUE, eyeViewMatrix.M[0]);
			glUniformMatrix4fv(_Prog->uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0]);

			CurrentManga->Draw(Matrix4f::Identity());
		}

		glBindVertexArray(0);
		glUseProgram(0);

		frameParms.ExternalVelocity = Scene.GetExternalVelocity();
		frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

		return eyeViewProjection;
	}

	void MangaCarousel::SetManga(Manga *manga) {
		// TODO: Change logic
		CurrentManga = manga;
	}

}