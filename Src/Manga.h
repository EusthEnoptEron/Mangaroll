#pragma once

#include "App.h"
#include "GlObject.h"
#include "Page.h"
#include "AsyncTexture.h"

using namespace OVR;

namespace OvrMangaroll {

	class MangaProvider;
	
	class IBrowserObject {
	public:
		virtual ~IBrowserObject() {}
		String UID;
	};

	class Manga : public GlObject, public IBrowserObject
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
		void IncreaseAngleOffset(float deg);
	protected:
		virtual void _Init() { }
		virtual void _Update() { }
		void ApplyAngleOffset();
		bool _Initialized;
		GlProgram _Prog;
		int _Count;
		Page *_First;
		Page *_Last;
		Page *_Selection;
		int _SelectionIndex;
		float _Angle;
		int _Progress;
		float _AngleOffset;
		int _LastSetPage;
	};

	class RemoteManga : public Manga {
	public:
		RemoteManga();
		virtual ~RemoteManga() {}
		String ID;
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


	enum ArchiveType {
		ARCHIVE_ZIP,
		ARCHIVE_RAR
	};
	class ArchivedManga : public Manga {
	public:
		ArchivedManga(String filePath);
		virtual ~ArchivedManga() {}
		String GetThumb() {
			return _Count > 0 ? GetPage(0).GetPath() : "";
		}
		bool IsValid() { return _Valid; }

		static bool IsArchiveFile(String ext) {
			return IsRAR(ext) || IsZIP(ext);
		}
		static bool IsRAR(String ext) {
			return ext.CompareNoCase(".rar") == 0
				|| ext.CompareNoCase(".cbr") == 0;
		}
		static bool IsZIP(String ext) {
			return ext.CompareNoCase(".zip") == 0
				|| ext.CompareNoCase(".cbz") == 0
				|| ext.CompareNoCase(".epub") == 0;
		}
	private:
		
		void Populate();
		String _Path;
		bool _Valid;
		ArchiveType _ArchiveType;

	};

	class MangaWrapper {
	public:
		MangaWrapper(MangaProvider *wrapper);
		MangaWrapper(Manga *manga);
		virtual ~MangaWrapper() {}

		void SetThumb(String path);
		String GetThumb() { return _Cover->GetPath(); }
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
