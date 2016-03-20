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
		, _BasePath("")
	{
		UID = "Local";
	}

	LocalMangaProvider::LocalMangaProvider(String basePath)
		: MangaProvider()
		, _Initialized(false)
		, _Mangas()
		, _BasePath(basePath)
	{
	}

	void LocalMangaProvider::LoadMore() {
		_Initialized = true;
	
		if (_BasePath.IsEmpty()) {
			// SEARCH FOR MANGA
			LOG("Looking for manga...");
			const OvrStoragePaths & paths = AppState::Instance->GetStoragePaths();

			Array<String> SearchPaths;
			paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
			paths.PushBackSearchPathIfValid(EST_SECONDARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);
			paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "RetailMedia/", SearchPaths);
			paths.PushBackSearchPathIfValid(EST_PRIMARY_EXTERNAL_STORAGE, EFT_ROOT, "", SearchPaths);

			StringHash<String> results = RelativeDirectoryFileList(SearchPaths, "Manga/");
			String mangaPath;
			LOG("Found %d manga folders", results.GetSize());
			// Load all mangas...
			for (StringHash<String>::Iterator it = results.Begin(); it != results.End(); ++it) {
				if (it->Second.GetCharAt(it->Second.GetLengthI() - 1) == '/') {
					LOG("Looking into %s", it->Second.ToCStr());
					mangaPath = GetFullPath(SearchPaths, it->Second);
					ExploreDirectory(mangaPath);
				}
			}
			LOG("Done looking.");
		}
		else {
			// We have a base path to explore
			Array<String> files = DirectoryFileList(_BasePath.ToCStr());

			for (int i = 0; i < files.GetSizeI(); i++) {
				if (files[i].Right(1) == "/") {
					ExploreDirectory(files[i]);
				}
			}
		}
	}

	void LocalMangaProvider::ExploreDirectory(String dir) {
		Array<String> files = DirectoryFileList(dir.ToCStr());
		LOG("Exploring directory: %s", dir.ToCStr());
		bool isManga = false;
		bool isContainer = false;

		// Try to parse as manga
		for (int i = 0; i < files.GetSizeI(); i++) {
			if (files[i].Right(1) == "/") {
				isManga = false;
				isContainer = true;
				break;
			}

			if (IsSupportedExt(files[i].GetExtension())) {
				isManga = true;
			}
		}
		
		LOG("Found... %s", isManga ? "Manga" : (isContainer ? "Container" : "Nothing"));

		if (isManga) {
			Manga *manga = new Manga();
			manga->Name = ExtractDirectory(dir);
			manga->UID = BuildUID(manga->Name);
			LOG("Add manga: %s", manga->Name.ToCStr());
			String cover("");
			for (int i = 0; i < files.GetSizeI(); i++) {
				if (IsSupportedExt(files[i].GetExtension())) {
					
					manga->AddPage(new LocalPage(files[i]));

					if (cover.IsEmpty()) {
						cover = files[i];
					}
				}
			}

			MangaWrapper *wrapper = new MangaWrapper(manga);
			wrapper->SetThumb(cover);
			wrapper->Name = manga->Name;
			_Mangas.PushBack(wrapper);
		}
		else if (isContainer) {
			LocalMangaProvider *subProvider = new LocalMangaProvider(dir);
			subProvider->LoadMore();
			if (subProvider->HasManga()) {
				MangaWrapper *wrapper = new MangaWrapper(subProvider);
				wrapper->SetThumb(subProvider->GetFirstManga()->GetThumb());
				wrapper->Name = ExtractDirectory(dir);
				subProvider->UID = BuildUID(wrapper->Name);
				_Mangas.PushBack(wrapper);
			}
			else {
				delete subProvider;
			}
		}
	}

	bool LocalMangaProvider::HasManga() {
		return _Mangas.GetSizeI() > 0;
	}

	MangaWrapper *LocalMangaProvider::GetFirstManga() {
		return _Mangas.Front();
	}


	// ############### SERVICE PROVIDER IMPLEMENTATION #############
	MangaServiceProvider::MangaServiceProvider()
		: MangaProvider()
		, _Initialized(false)
		, _Services()
	{
		UID = "Online";
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
								provider->UID = BuildUID(provider->Name);
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
		String url = ParamString::InsertParam(_BrowseUrl.ToCStr(), ParamString::PARAM_PAGE, _Page);
		url = ParamString::InsertParam(url.ToCStr(), ParamString::PARAM_ID, Id.ToCStr());

		Web::Download(url,
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
								container->Id = mangaReader.GetChildStringByName("id");
								container->UID = provider->BuildUID(container->Id);
								wrapper->Name = mangaReader.GetChildStringByName("name");
								container->Name = mangaReader.GetChildStringByName("name");
								wrapper->SetThumb(mangaReader.GetChildStringByName("thumb"));
								provider->_MangasBuffer.PushBack(wrapper);
							} else {
								RemoteManga *manga = new RemoteManga();
								MangaWrapper *wrapper = new MangaWrapper(manga);

								manga->Name = mangaReader.GetChildStringByName("name");
								manga->ID = mangaReader.GetChildStringByName("id");
								manga->UID = provider->BuildUID(manga->ID);
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