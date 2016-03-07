#pragma once

#include "App.h"
#include "GlObject.h"
#include "Page.h"
#include "AsyncTexture.h"

using namespace OVR;

namespace OvrMangaroll {

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
		void SetProgress(int page);
		int GetCount(void);
		Page &GetPage(int);
		String Name;
		bool Selectionable;
		AsyncTexture *GetCover();
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
		AsyncTexture *_Cover;
		int _AngleOffset;
	};

	class RemoteManga : public Manga {
	public:
		RemoteManga();
		virtual ~RemoteManga() {}
		int ID;
		void SetThumb(String url);
		String FetchUrl;
	protected:
		virtual void _Init();
		virtual void _Update();

	private:
		static void OnDownload(void *buffer, int length, void *p);
		Array<Page *> _Payload;
		bool _Loading;
	};

}
