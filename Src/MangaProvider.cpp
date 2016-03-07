#include "Mangaroll.h"
#include "MangaProvider.h"
#include "Helpers.h"
#include "jni.h"
#include "Android\JniUtils.h"

namespace OvrMangaroll {

	// ############### LOCAL PROVIDER IMPLEMENTATION #############
	LocalMangaProvider::LocalMangaProvider()
		: MangaProvider()
		, _Initialized(false)
		, _Mangas()
	{
	}

	void LocalMangaProvider::LoadMore() {
		_Initialized = true;

		// SEARCH FOR MANGA
		const OvrStoragePaths & paths = AppState::Instance->GetStoragePaths();

		Array<String> SearchPaths;
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

		StringHash<String> results = RelativeDirectoryFileList(SearchPaths, "Manga/");
		String mangaPath;

		// Load all mangas...
		for (StringHash<String>::Iterator it = results.Begin(); it != results.End(); ++it) {
			if (it->Second.GetCharAt(it->Second.GetLengthI() - 1) == '/') {
				mangaPath = GetFullPath(SearchPaths, it->Second);
				Manga *manga = new Manga();

				Array<String> images = DirectoryFileList(mangaPath.ToCStr());

				manga->Name = ExtractDirectory(mangaPath);
				for (int i = 0; i < images.GetSizeI(); i++) {
					//WARN("%s -> %s", images[i].ToCStr(), images[i].GetExtension().ToCStr());
					if (images[i].GetExtension() == ".jpg") {
						manga->AddPage(new LocalPage(images[i]));
					}
				}

				_Mangas.PushBack(manga);
			}
		}
	}


	// ############## REMOTE PROVIDER IMPLEMENTATION ###########

	RemoteMangaProvider::RemoteMangaProvider(String browseUrl, String showUrl)
		: MangaProvider()
		, _Loading(false)
		, _HasMore(true)
		, _Page(0)
		, _BrowseUrl(browseUrl)
		, _ShowUrl(showUrl)
		, _Thread(NULL)
		, _Mangas()
	{
	}

	// Acts as update function
	bool RemoteMangaProvider::IsLoading() {
		if (_Loading && _Thread->IsFinished()) {
			_Loading = false;

			// Transfer mangas
			for (int i = 0; i < _MangasBuffer.GetSizeI(); i++) {
				_Mangas.PushBack(_MangasBuffer[i]);
			}
			_MangasBuffer.Clear();
		}
		return _Loading;
	}


	void RemoteMangaProvider::LoadMore() {
		_Thread = Web::Download(String::Format(_BrowseUrl.ToCStr(), _Page),
			RemoteMangaProvider::FetchFn
			, this);
		_Loading = true;

		// In case thread fails. Isn't used while loading.
		_HasMore = false;
	}


	void RemoteMangaProvider::FetchFn(void *buffer, int length, void *target) {
		RemoteMangaProvider *provider = (RemoteMangaProvider *)target;
		JSON *jsonFile = JSON::Parse((const char *)buffer);

		if (jsonFile == NULL) {
			provider->_HasMore = false;
		}
		else {
			JsonReader reader(jsonFile);
			if (reader.GetChildBoolByName("success")) {
				// Well, way to go!
				provider->_HasMore = reader.GetChildBoolByName("hasMore");
				provider->_Page += 1;
				
				// On to the mangas...
				JsonReader listReader(reader.GetChildByName("mangaList"));
				if (listReader.IsValid() && listReader.IsArray()) {
					while (!listReader.IsEndOfArray()) {
						JsonReader mangaReader(listReader.GetNextArrayElement());
						if (mangaReader.IsValid() && mangaReader.IsObject()) {
							RemoteManga *manga = new RemoteManga();
							manga->Name = mangaReader.GetChildStringByName("name");
							manga->ID = mangaReader.GetChildInt32ByName("id");
							manga->SetThumb(mangaReader.GetChildStringByName("thumb"));
							manga->FetchUrl = provider->_ShowUrl;

							provider->_MangasBuffer.PushBack(manga);
						}
					}
				}

			}
			else {
				provider->_HasMore = false;
			}
		}

		delete jsonFile;
	}
	
}