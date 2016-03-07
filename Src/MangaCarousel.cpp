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
		, _Mangaroll(app)
		, _PrevLookAt(0, 0, -1.0f)
		, _Angle(0)
		, _LastPress(0)
		, _Fader(1)
		, _Operatable(true)
	{
	}

	MangaCarousel::~MangaCarousel()
	{
	}

	void MangaCarousel::OneTimeInit(const char * launchIntent) {
		_Prog = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
		glUseProgram(_Prog->program);
		glUniform1f(glGetUniformLocation(_Prog->program, "Contrast"), 1.0f);
		glUniform1f(glGetUniformLocation(_Prog->program, "Brightness"), 0.0f);


		const char * scenePath = "assets/dojo_scene.ovrscene";

		MaterialParms materialParms;
		materialParms.UseSrgbTextureFormats = false;
		Scene.LoadWorldModelFromApplicationPackage(scenePath, materialParms);
		Scene.SetYawOffset(-Mathf::Pi * 0.5f);
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

		if (_Operatable) {
			// Only update this when operatable
			if (vrFrame.Input.buttonState & BUTTON_TOUCH_SINGLE) {
				AppState::Guide = (GuideType)((AppState::Guide + 1) % 3);
				_LastPress = Time::Elapsed;
			}
		}

		if (CurrentManga != NULL) {
			CurrentManga->Selectionable = _Operatable;
			CurrentManga->Update(_Angle, false);
		}

		Scene.Frame(vrFrame, _Mangaroll->app->GetHeadModelParms());
		_Fader.Update(3, Time::Delta);

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
		const Matrix4f eyeViewProjection = Scene.DrawEyeView(eye, fovDegreesX, fovDegreesY);


		if (CurrentManga != NULL) {
			glUseProgram(_Prog->program);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniformMatrix4fv(_Prog->uView, 1, GL_TRUE, eyeViewMatrix.M[0]);
			glUniformMatrix4fv(_Prog->uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0]);

			CurrentManga->Draw(Matrix4f::Scaling((1-_Fader.GetFadeAlpha()) + 1));
		}

		glBindVertexArray(0);
		glUseProgram(0);

		frameParms.ExternalVelocity = Scene.GetExternalVelocity();
		frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;

		return eyeViewProjection;
	}

	void MangaCarousel::SetManga(Manga *manga) {
		if (CurrentManga != NULL) {
			CurrentManga->Unload();
		}
		CurrentManga = manga; 
		manga->Init();

		int progress = CurrentManga->GetProgress();
		CurrentManga->Update(_Angle, true);
		CurrentManga->SetProgress(progress);
	}

	void MangaCarousel::MoveOut(void) {
		_Operatable = false;

		_Fader.StartFadeOut();
	}

	void MangaCarousel::MoveIn(void) {
		_Operatable = true;

		_Fader.StartFadeIn();
	}
}