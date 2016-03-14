#pragma once

#include "App.h"
#include "GlObject.h"
#include "Page.h"
#include "AsyncTexture.h"

using namespace OVR;

namespace OvrMangaroll {

	class MangaProvider;

	class Manga : public GlObject
	{
	public:
		Manga(void);
		virtual ~Manga(void);
		void Init() {
			if (!_Initialized) {
				_Init();
				_Initialized = true;
			}
		}
		void Draw(const Matrix4f&);
		void AddPage(Page *page);
		void Update(float angle, bool onlyVisual = false);
		int GetProgress(void);
		void SetProgress(int page, float angleToAppear = FLT_MAX);
		int GetCount(void);
		Page &GetPage(int);
		String Name;
		bool Selectionable;
		void Unload();
	protected:
		virtual void _Init() { }
		virtual void _Update() { }
		bool _Initialized;
		GlProgram _Prog;
		int _Count;
		Page *_First;
		Page *_Last;
		Page *_Selection;
		int _SelectionIndex;
		float _Angle;
		int _Progress;
		int _AngleOffset;
	};

	class RemoteManga : public Manga {
	public:
		RemoteManga();
		virtual ~RemoteManga() {}
		int ID;
		String FetchUrl;
	protected:
		virtual void _Init();
		virtual void _Update();

	private:
		void Fetch();
		static void OnDownload(void *buffer, int length, void *p);
		Array<Page *> _Payload;
		bool _Loading;
		bool _HasMore;
		int _Page;
	};


	class MangaWrapper {
	public:
		MangaWrapper(MangaProvider *wrapper);
		MangaWrapper(Manga *manga);
		virtual ~MangaWrapper() {}

		void SetThumb(String path);
		AsyncTexture *GetCover() { return _Cover; }
		MangaProvider *GetContainer() { return _Category; }
		Manga *GetManga() { return _Manga; }
		bool IsContainer() { return _Category != NULL; }
		bool IsManga() { return _Manga != NULL; }
		String Name;
	private:
		Manga *_Manga;
		MangaProvider *_Category;
		AsyncTexture *_Cover;
	};

}
