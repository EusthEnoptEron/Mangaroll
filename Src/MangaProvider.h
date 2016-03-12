#pragma once

#include "Kernel\OVR_Array.h"
#include "Kernel\OVR_JSON.h"
#include "Manga.h"
#include "Page.h"

using namespace OVR;
namespace OvrMangaroll {
	
	// Interface for providers of manga
	class MangaProvider {
	public:
		virtual ~MangaProvider() {}

		virtual int GetCurrentSize() = 0;
		virtual bool HasMore() = 0;
		virtual void LoadMore() = 0;
		virtual bool IsLoading() = 0;
		virtual MangaWrapper *At(int i) = 0;
	protected:
		MangaProvider() : Id("") {}
		String Id;
	};


	// Provider that looks for manga locally
	class LocalMangaProvider : public MangaProvider {
	public:
		LocalMangaProvider();
		virtual ~LocalMangaProvider() {}

		// Implement
		virtual bool HasMore() { return !_Initialized; }
		virtual bool IsLoading() { return false; }
		virtual MangaWrapper *At(int i) { return _Mangas.At(i); }
		virtual int GetCurrentSize() { return _Mangas.GetSizeI(); }

		virtual void LoadMore();
	private:
		bool _Initialized;
		Array<MangaWrapper *> _Mangas;
	};

	class RemoteMangaProvider : public MangaProvider {
	public:
		RemoteMangaProvider(String browseUrl, String showUrl);
		virtual ~RemoteMangaProvider() {}

		// Implement
		virtual bool HasMore() { return _HasMore; }
		virtual MangaWrapper *At(int i) { return _Mangas.At(i); }
		virtual int GetCurrentSize() { return _Mangas.GetSizeI(); }

		virtual bool IsLoading();
		virtual void LoadMore();
		String Name;
	private:
		bool _Loading;
		bool _HasMore;
		int _Page;
		String _BrowseUrl;
		String _ShowUrl;
		bool _DoneReading;
		Array<MangaWrapper *> _Mangas;
		Array<MangaWrapper *> _MangasBuffer;

		
		static void FetchFn(void *buffer, int length, void *target);
	};

}