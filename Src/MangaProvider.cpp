#include "Mangaroll.h"
#include "MangaProvider.h"
#include "Helpers.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "Web.h"

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
				String cover("");
				for (int i = 0; i < images.GetSizeI(); i++) {
					//WARN("%s -> %s", images[i].ToCStr(), images[i].GetExtension().ToCStr());
					if (images[i].GetExtension() == ".jpg") {
						manga->AddPage(new LocalPage(images[i]));

						if (cover.IsEmpty()) {
							cover = images[i];
						}
					}
				}

				MangaWrapper *wrapper = new MangaWrapper(manga);
				wrapper->SetThumb(cover);
				wrapper->Name = manga->Name;
				_Mangas.PushBack(wrapper);
			}
		}
	}

	// ############### SERVICE PROVIDER IMPLEMENTATION #############
	MangaServiceProvider::MangaServiceProvider()
		: MangaProvider()
		, _Initialized(false)
		, _Services()
	{
	}

	void MangaServiceProvider::LoadMore() {
		_Initialized = true;

		// LOAD CONFIGURED REMOTE SITES
		const OvrStoragePaths & paths = AppState::Instance->GetStoragePaths();

		Array<String> SearchPaths;
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
		paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

		for (int i = 0; i < SearchPaths.GetSizeI(); i++) {
			const char *path = (SearchPaths[i] + "Manga/services.json").ToCStr();
			if (FileExists(path)) {
				JSON *jsonFile = JSON::Load(path);
				if (jsonFile != NULL) {
					JsonReader serviceReader(jsonFile);
					if (serviceReader.IsValid() && serviceReader.IsArray()) {
						while (!serviceReader.IsEndOfArray()) {
							JsonReader elementReader(serviceReader.GetNextArrayElement());
							if (elementReader.IsValid() && elementReader.IsObject()) {
								RemoteMangaProvider *provider = new RemoteMangaProvider(
									elementReader.GetChildStringByName("browseUrl"),
									elementReader.GetChildStringByName("showUrl")
									);
								provider->Name = elementReader.GetChildStringByName("name");
								MangaWrapper *wrapper = new MangaWrapper(provider);
								wrapper->Name = provider->Name;
								wrapper->SetThumb("");
								// TODO: icon
								_Services.PushBack(wrapper);
							}
						}
					}
					delete jsonFile;
				}
				//break;
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
		, _Mangas()
	{
	}

	// Acts as update function
	bool RemoteMangaProvider::IsLoading() {
		if (_Loading && _DoneReading) {//_Thread->IsFinished()) {
			_Loading = false;
			_DoneReading = false;
			// Transfer mangas
			for (int i = 0; i < _MangasBuffer.GetSizeI(); i++) {
				_Mangas.PushBack(_MangasBuffer[i]);
			}
			_MangasBuffer.Clear();
		}
		return _Loading;
	}


	void RemoteMangaProvider::LoadMore() {
		Web::Download(String::Format(_BrowseUrl.ToCStr(), _Page, Id.ToCStr()),
			RemoteMangaProvider::FetchFn
			, this);
		_Loading = true;
		_DoneReading = false;
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
							if (mangaReader.GetChildBoolByName("container")) {
								RemoteMangaProvider *container = new RemoteMangaProvider(provider->_BrowseUrl, provider->_ShowUrl);
								MangaWrapper *wrapper = new MangaWrapper(container);
								container->Id = String::Format("%d", mangaReader.GetChildInt32ByName("id"));
								wrapper->Name = mangaReader.GetChildStringByName("name");
								container->Name = mangaReader.GetChildStringByName("name");
								wrapper->SetThumb(mangaReader.GetChildStringByName("thumb"));
								provider->_MangasBuffer.PushBack(wrapper);
							} else {
								RemoteManga *manga = new RemoteManga();
								MangaWrapper *wrapper = new MangaWrapper(manga);

								manga->Name = mangaReader.GetChildStringByName("name");
								manga->ID = mangaReader.GetChildInt32ByName("id");
								manga->FetchUrl = provider->_ShowUrl;
								wrapper->SetThumb(mangaReader.GetChildStringByName("thumb"));
								wrapper->Name = manga->Name;
								provider->_MangasBuffer.PushBack(wrapper);
							}
							
						}
					}
				}

			}
			else {
				provider->_HasMore = false;
			}
		}

		delete jsonFile;

		provider->_DoneReading = true;
	}
	
}