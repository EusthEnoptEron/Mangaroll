#include "Helpers.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "VrApi.h"
#include "Kernel\OVR_Threads.h"
#include "App.h"
#include "Kernel\OVR_Math.h"
#include "Mangaroll.h"

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

	App *AppState::Instance = NULL;
	jclass AppState::FetcherClass = NULL;
	jclass AppState::ContainerFetcherClass = NULL;
	jclass AppState::MangaFetcherClass = NULL;
	jclass AppState::ActivityClass = NULL;
	Config *AppState::Conf = NULL;
	Mangaroll *AppState::Reader = NULL;
	ThreadPool *AppState::Scheduler = NULL;

	Assets *Assets::_Instance = NULL;


	String MangaStringUtils::CropToLength(String base, int maxLength) {
		if (base.GetLengthI() < maxLength) {
			return base;
		}

		base = base.Left(maxLength - 3);
		return base + "...";
	}

	const char * const ParamString::PARAM_ID = "{id}";
	const char * const ParamString::PARAM_PAGE = "{page}";

	String ParamString::InsertParam(const char *source, const char *paramName, const char *value) {
		const ovrJava  *java = AppState::Instance->GetJava();
		JNIEnv *env = java->Env;
		JavaClass clazz = JavaClass(env, env->GetObjectClass(java->ActivityObject));

		jmethodID _InsertParam = env->GetStaticMethodID(clazz.GetJClass(), "InsertParam", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

		// Prepare params
		JavaString src = JavaString(env, source);
		JavaString param = JavaString(env, paramName);
		JavaString jvalue = JavaString(env, value);	
		JavaUTFChars result = JavaUTFChars(env, (jstring)env->CallStaticObjectMethod(clazz.GetJClass(), _InsertParam, src.GetJString(), param.GetJString(), jvalue.GetJString()));
		String resultString(result.ToStr());
		return resultString;
	}

	
	Vector3f Color::HSL2RGB(const Vector3f &hsl) {
		//Catch malformed H input for H(0 to 1)
		//if (hsl.x < 0)
		//	hsl.x = 0;

		//else if (hsl.x > 1.0)
		//	hsl.x = 1.0;
		

		//Catch malformed S input
		//if (hsl.y < 0)
		//	hsl.y = 0;

		//else if (hsl.y > 1.0)
		//	hsl.y = 1.0;

		////Catch malformed L input
		//if (hsl.z < 0)
		//	hsl.z = 0;

		//else if (hsl.z > 1.0)
		//	hsl.z = 1.0;

		//Declare required variables
		Vector3f rgb;
		float q;
		float p;
		float tr;
		float tg;
		float tb;

		//Special case: When S = 0, the result is monochromatic, and R = B = G = L.
		if (hsl.y == 0){
			rgb.x = rgb.y = rgb.z = hsl.z;
			return rgb;
		}

		//Set up temporary values for conversion
		if (hsl.z < 0.5)
			q = hsl.z * (1.0 + hsl.y);

		else if (hsl.z >= 0.5)
			q = hsl.z + hsl.y - (hsl.z * hsl.y);

		p = 2 * hsl.z - q;

		tr = hsl.x + (1.0 / 3.0);
		tg = hsl.x;
		tb = hsl.x - (1.0 / 3.0);

		//Normalize temporary R value
		if (tr < 0)
			tr = tr + 1.0;

		else if (tr > 1.0)
			tr = tr - 1.0;

		//Normalize temporary G value
		if (tg < 0)
			tg = tg + 1.0;

		else if (tg > 1.0)
			tg = tg - 1.0;

		//Normalize temporary B value
		if (tb < 0)
			tb = tb + 1.0;

		else if (tb > 1.0)
			tb = tb - 1.0;


		//Calculate R value
		if (tr < (1.0 / 6.0))
			rgb.x = p + ((q - p) * 6 * tr);

		else if ((1.0 / 6.0) <= tr && tr < 0.5)
			rgb.x = q;

		else if (0.5 <= tr && tr < (2.0 / 3.0))
			rgb.x = p + ((q - p) * 6 * ((2.0 / 3.0) - tr));

		else
			rgb.x = p;

		//Calculate G value
		if (tg < (1.0 / 6.0))
			rgb.y = p + ((q - p) * 6 * tg);

		else if ((1.0 / 6.0) <= tg && tg < 0.5)
			rgb.y = q;

		else if (0.5 <= tg && tg < (2.0 / 3.0))
			rgb.y = p + ((q - p) * 6 * ((2.0 / 3.0) - tg));

		else
			rgb.y = p;

		//Caluclate B value
		if (tb < (1.0 / 6.0))
			rgb.z = p + ((q - p) * 6 * tb);

		else if ((1.0 / 6.0) <= tb && tb < 0.5)
			rgb.z = q;

		else if (0.5 <= tb && tb < (2.0 / 3.0))
			rgb.z = p + ((q - p) * 6 * ((2.0 / 3.0) - tb));

		else
			rgb.z = p;

		//Return the result
		return rgb;
	}

	Vector3f Color::RGB2HSL(const Vector3f &rgb) {
		float r = rgb.x;
		float g = rgb.y;
		float b = rgb.z;
		float h;
		float s;
		float l;
		float max;
		float min;

		// Looking for the max value among r, g and b
		if (r > g && r > b) max = r;
		else if (g > b) max = g;
		else max = b;

		// Looking for the min value among r, g and b
		if (r < g && r < b) min = r;
		else if (g < b) min = g;
		else min = b;

		l = (max + min) / 2.0;

		if (max == min)
		{
			h = 0.0;
			s = 0.0;
		}
		else
		{
			float d = max - min;

			if (l > 0.5) s = d / (2.0 - max - min);
			else s = d / (max + min);

			if (max == r) {
				if (g < b) h = (g - b) / d + 6.0;
				else h = (g - b) / d;
			}
			else if (max == g)
				h = (b - r) / d + 2.0;
			else
				h = (r - g) / d + 4.0;
			h /= 6.0;
		}

		return Vector3f(h, s, l);
	}

}