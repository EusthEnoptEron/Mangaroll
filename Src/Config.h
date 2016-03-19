#pragma once
#ifndef ANDROID
#define ANDROID
#endif

#include "Helpers.h"
#include "Kernel\OVR_JSON.h"
#include "Kernel\OVR_Hash.h"
#include "Kernel\OVR_StringHash.h"
#include "jni.h"

using namespace OVR;

namespace OvrMangaroll {
	class Manga;

	struct MangaInfo {
		int PagesRead;
		int PagesTotal;

		MangaInfo(int pagesRead, int pagesTotal = -1)
			: PagesRead(pagesRead)
			, PagesTotal(pagesTotal)
		{

		}
	};

	class Config {
	public:
		static Config *Load(App *);
		static Config *Load(String path);
		void Save();

		MangaInfo GetProgress(Manga *manga) const;
		void Persist(Manga *manga);
		bool HasInfo(Manga *manga) const;

	public:
		// When you add a property, make sure they are properly serialized and deserialized
		bool Guided;
		bool Transparent;
		bool LeftToRight;
		bool AutoRotate;
		float Contrast;
		float Brightness;
		float Zoom;
		void ClearProgress();

	private:
		Config(String path);
		Config(const Config &conf);
		void LoadJSON();
		
		void DeserializeConfigs(const JsonReader &);
		void DeserializeReadingStates(const JsonReader &);
		void SerializeConfigs(JSON *);
		void SerializeReadingStates(JSON *);

	private:
		String _Path;
		StringHash<MangaInfo> _Progress;

	};
}

