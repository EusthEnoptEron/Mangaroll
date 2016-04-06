#include "MangaCarousel.h"
#include "Helpers.h"
#include "ShaderManager.h"
#include "Mangaroll.h"

#include "GLES3\gl3_loader.h"
#include "Config.h"



namespace OvrMangaroll {

	const float MangaCarousel::SCROLL_ANGLE_MIN = 5.0f;
	const float MangaCarousel::SCROLL_ANGLE_MAX = 90.0f;
	const float MangaCarousel::SCROLL_SPEED_MIN = 0.0f;
	const float MangaCarousel::SCROLL_SPEED_MAX = 100.0f;


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
		, _Progs()
		, _uContrast()
		, _uBrightness()
		, _CenterEyeViewMatrix()
		, _Mangaroll(app)
		, _PrevLookAt(0, 0, -1.0f)
		, _Angle(0)
		, _LastPress(0)
		, _Fader(1)
		, _Operatable(true)
		, _Scaling(false)
		, _ForwardAngle(0)
		, _AngleAnimator(Interpol::CubicEaseInOut)
	{
	}

	MangaCarousel::~MangaCarousel()
	{
	}

	void MangaCarousel::OneTimeInit(const char * launchIntent) {
		_Progs[0] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME);
		_Progs[1] = ShaderManager::Instance()->Get(PAGE_SHADER_NAME, PAGE_TRANSPARENT_FRAG_NAME);

		_uContrast[0] = glGetUniformLocation(_Progs[0]->program, "Contrast");
		_uContrast[1] = glGetUniformLocation(_Progs[1]->program, "Contrast");
		_uBrightness[0] = glGetUniformLocation(_Progs[0]->program, "Brightness");
		_uBrightness[1] = glGetUniformLocation(_Progs[1]->program, "Brightness");

		//const char * scenePath = "assets/default.ovrscene";
		const char * scenePath = "assets/stars.ovrscene";

		MaterialParms materialParms;
		materialParms.UseSrgbTextureFormats = false;
		Scene.LoadWorldModelFromApplicationPackage(scenePath, materialParms);
	}

	void MangaCarousel::OneTimeShutdown(){
	}

	Matrix4f MangaCarousel::Frame(const VrFrame & vrFrame){
		_CenterEyeViewMatrix = vrapi_GetCenterEyeViewMatrix(&_Mangaroll->app->GetHeadModelParms(), &vrFrame.Tracking, NULL);

		float angleA = RadToDegree(atan2(_PrevLookAt[0], _PrevLookAt[2]));
		float angleB = RadToDegree(atan2(HMD::Direction[0], HMD::Direction[2]));
		float angleDiff = deltaAngle(angleA, angleB);
		//angleDiff *= -1;
		_Angle -= AppState::Conf->LeftToRight ? -angleDiff : angleDiff;

		_PrevLookAt = HMD::Direction;

		if (_Operatable) {
			// Only update this when operatable
			/*if (vrFrame.Input.buttonState & BUTTON_TOUCH_SINGLE) {
				AppState::Guide = (GuideType)((AppState::Guide + 1) % 3);
				_LastPress = Time::Elapsed;
			}*/

			// ---- HANDLE ZOOM ----
			if (vrFrame.Input.buttonPressed & BUTTON_TOUCH) {
				_Scaling = true;
				_StartZoom = AppState::Conf->Zoom;
			}
			if (_Scaling) {
				AppState::Conf->Zoom = Alg::Clamp(_StartZoom + (vrFrame.Input.touchRelative.y / 500.0f), 0.0f, 1.0f);
			}
			// ---------------------
			

			// ---- HANDLE AUTO-ROTATE ----
			if (CurrentManga != NULL && AppState::Conf->AutoRotate) {
				// We might have to auto-rotate
				float deltaAngle = _ForwardAngle - _Angle;
				float absAngle = Alg::Abs(deltaAngle);
				if (absAngle >= SCROLL_ANGLE_MIN) {
					float speedMix = Alg::Clamp( (absAngle - SCROLL_ANGLE_MIN) / (SCROLL_ANGLE_MAX - SCROLL_ANGLE_MIN), 0.0f, 1.0f);
					float speed = Alg::Lerp(SCROLL_SPEED_MIN, SCROLL_SPEED_MAX, speedMix) * (deltaAngle < 0 ? 1 : -1);
					LOG("Moving at %.2f deg/s", speed);
					CurrentManga->IncreaseAngleOffset(speed * Time::Delta);
				}
			}
			// -----------------------------

			// ---- HANDLE SWIPE ----

			if (!_AngleAnimator.IsActive() && (vrFrame.Input.buttonState & BUTTON_SWIPE_FORWARD || vrFrame.Input.buttonState & BUTTON_SWIPE_BACK)) {
				bool goForward = vrFrame.Input.buttonState & BUTTON_SWIPE_FORWARD;
				Page *currentPage = CurrentManga->GetCurrentPage();
				if (currentPage != NULL && currentPage->GetAngle() != RANGE_UNDEFINED) {
					Page *otherPage = goForward ? currentPage->GetNext() : currentPage->GetPrev();
					if (otherPage != NULL && otherPage->GetAngle() != RANGE_UNDEFINED) {
						float angleRange = (currentPage->GetAngle() + otherPage->GetAngle()) * 0.5f;
						if (angleRange != RANGE_UNDEFINED) {
							_AngleAnimator.Start(Time::Elapsed, 0.2f,
								CurrentManga->GetAngleOffset(),
								CurrentManga->GetAngleOffset() + (goForward
								? angleRange
								: -angleRange));
						}
					}
				}
			}

			// ----------------------
		}
		if (vrFrame.Input.buttonReleased & BUTTON_TOUCH) {
			_Scaling = false;
		}

		if (CurrentManga != NULL) {
			// Handle swipe
			if (_AngleAnimator.IsActive()) {
				_AngleAnimator.Update(Time::Elapsed);
				CurrentManga->SetAngleOffset(_AngleAnimator.Value());
			}

			CurrentManga->Selectionable = _Operatable;
			CurrentManga->Update(_Angle, false);
		}


		Scene.Frame(vrFrame, _Mangaroll->app->GetHeadModelParms());
		_Fader.Update(3, Time::Delta);

		return _CenterEyeViewMatrix;
	}

	void MangaCarousel::ChangeDirection() {
		_Angle = -_Angle;

		if (CurrentManga != NULL) {
			CurrentManga->SetProgress(CurrentManga->GetProgress());
		}
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

		int idx = AppState::Conf->Transparent ? 1 : 0;
		_Prog = _Progs[idx];

		if (CurrentManga != NULL) {
			glUseProgram(_Prog->program);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniformMatrix4fv(_Prog->uView, 1, GL_TRUE, eyeViewMatrix.M[0]);
			glUniformMatrix4fv(_Prog->uProjection, 1, GL_TRUE, eyeProjectionMatrix.M[0]);
			glUniform1f(_uContrast[idx], AppState::Conf->Contrast);
			glUniform1f(_uBrightness[idx], AppState::Conf->Brightness);

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
			AppState::Conf->Persist(CurrentManga);
			CurrentManga->Unload();
		}
		CurrentManga = manga; 
		manga->Init();
		
		CurrentManga->Update(_Angle, true);
		CurrentManga->SetProgress(AppState::Conf->GetProgress(manga).PagesRead);
		LOG("Loaded manga: %s at p%d", manga->Name.ToCStr(), AppState::Conf->GetProgress(manga).PagesRead);
		CurrentManga->Position = Vector3f(0,-0.1f,0); // Move down by a bit
	}

	void MangaCarousel::MoveOut(void) {
		_Operatable = false;
		if (CurrentManga != NULL) {
			AppState::Conf->Persist(CurrentManga);
		}

		_Fader.StartFadeOut();
	}

	void MangaCarousel::MoveIn(void) {
		_Operatable = true;

		_Fader.StartFadeIn();

		// This is the new forward
		_ForwardAngle = _Angle;
	}
}