#include "Helpers.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "VrApi.h"
#include "Kernel\OVR_Threads.h"
#include "App.h"
#include "Kernel\OVR_Math.h"

using namespace OVR;
namespace OvrMangaroll {
	double Time::Delta = 0;
	double Time::Elapsed = 0;

	const VrFrame *Frame::Current = NULL;

	Vector3f HMD::Direction = Vector3f(0, 0, -1.0f);
	//Vector3f HMD::Position = Vector3f(0, 0, 0);
	//Quatf HMD::Orientation = Quatf();

	float HMD::HorizontalAngle = 0;
	float HMD::VerticalAngle = 0;

	GuideType AppState::Guide = GuideType::NONE;
	App *AppState::Instance = NULL;
	bool AppState::Transparent = false;
	bool AppState::LeftToRight = false;
	bool AppState::AutoRotate = false;
	float AppState::Contrast = 1.0f;
	float AppState::Brightness = 0.0f;

	Assets *Assets::_Instance = NULL;


	String MangaStringUtils::CropToLength(String base, int maxLength) {
		if (base.GetLengthI() < maxLength) {
			return base;
		}

		base = base.Left(maxLength - 3);
		return base + "...";
	}
}