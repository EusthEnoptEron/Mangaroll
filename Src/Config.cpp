#include "Config.h"

namespace OvrMangaroll {

	Config::Config(String path)
		// DEFAULTS
		: Guided(true)
		, Transparent(false)
		, LeftToRight(false)
		, AutoRotate(false)
		, Contrast(1.0f)
		, Brightness(0.0f)
		, Zoom(0.0f)
		// ----
		, _Path(path)
		, _Progress()
	{
	
	}


	Config *Config::Load(App *app) {
		app->GetJava()->ActivityObject;
		JNIEnv *env = app->GetJava()->Env;
		JavaClass clazz(env, env->GetObjectClass(app->GetJava()->ActivityObject));
		jmethodID getConfigDir = env->GetMethodID(clazz.GetJClass(), "getConfigDir", "()Ljava/lang/String;");

		JavaUTFChars resultString(env, (jstring)env->CallObjectMethod(app->GetJava()->ActivityObject, getConfigDir));

		return Load(String(resultString.ToStr()) + "config.json");
	}

	Config *Config::Load(String path) {
		Config *config = new Config(path);

		config->LoadJSON();

		return config;
	}


	void Config::LoadJSON() {
		JSON *file = JSON::Load(_Path.ToCStr());
		ClearProgress();

		if (file == NULL) {
			WARN("Failed to load config file (%s)", _Path.ToCStr());
		}
		else {
			const JsonReader reader(file);
			if (reader.IsObject()) {
				// Get config values
				DeserializeConfigs(reader);
				DeserializeReadingStates(reader);
			}

			file->Release();
		}

	}

	void Config::ClearProgress() {
		//for (Hash<String, MangaInfo>::Iterator it = _Progress.Begin(); it != _Progress.End(); it++) {
		//	it->Second
		//}

		_Progress.Clear();

	}

	void Config::DeserializeConfigs(const JsonReader &reader) {
		Guided = reader.GetChildBoolByName("Guided", Guided);
		Transparent = reader.GetChildBoolByName("Transparent", Transparent);
		LeftToRight = reader.GetChildBoolByName("LeftToRight", LeftToRight);
		AutoRotate = reader.GetChildBoolByName("AutoRotate", AutoRotate);
		Contrast = reader.GetChildFloatByName("Contrast", Contrast);
		Brightness = reader.GetChildFloatByName("Brightness", Contrast);
		Zoom = reader.GetChildFloatByName("Zoom", Zoom);
	}

	void Config::SerializeConfigs(JSON *conf) {
		conf->AddBoolItem("Guided", Guided);
		conf->AddBoolItem("Transparent", Transparent);
		conf->AddBoolItem("LeftToRight", LeftToRight);
		conf->AddBoolItem("AutoRotate", AutoRotate);
		conf->AddNumberItem("Contrast", Contrast);
		conf->AddNumberItem("Brightness", Brightness);
		conf->AddNumberItem("Zoom", Zoom);
	}


	void Config::DeserializeReadingStates(const JsonReader &confReader) {

		const JsonReader reader(confReader.GetChildByName("progress"));

		// Get reading states
		if (reader.IsArray()) {
			while (!reader.IsEndOfArray()) {
				const JsonReader element(reader.GetNextArrayElement());
				if (element.IsArray()) {
					_Progress.Add(element.GetNextArrayString(), 
						MangaInfo(element.GetNextArrayInt32(), element.GetNextArrayInt32()));
				}
			}
		}
	}

	void Config::SerializeReadingStates(JSON *conf) {
		JSON * progress = JSON::CreateArray();
		conf->AddItem("progress", progress);
		
		for (Hash<String, MangaInfo>::Iterator it = _Progress.Begin(); it != _Progress.End(); ++it) {
			JSON *item = JSON::CreateArray();
			progress->AddArrayElement(item);

			item->AddArrayString(it->First.ToCStr());
			item->AddArrayNumber(it->Second.PagesRead);
			item->AddArrayNumber(it->Second.PagesTotal);
		}
	}


	void Config::Save() {
		LOG("Saving config to %s", _Path.ToCStr());
		JSON *conf = JSON::CreateObject();

		SerializeConfigs(conf);
		SerializeReadingStates(conf);

		if (!FileExists(_Path.ToCStr())) {
			// Make path
			MakePath(_Path.GetPath().ToCStr(), permissionFlags_t(PERMISSION_READ) | PERMISSION_WRITE);
		}
		
		if (!conf->Save(_Path.ToCStr())) {
			WARN("Couldn't save config file to %s!", _Path.ToCStr());
		}

		conf->Release();
	}

}