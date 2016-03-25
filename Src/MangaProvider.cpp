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
				else if (IsArchivedManga(it->Second.GetExtension())) {
					LOG("Found comic book: %s", it->Second.ToCStr());
					AddArchivedManga(GetFullPath(SearchPaths, it->Second));
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

	void LocalMangaProvider::AddArchivedManga(String file) {
		ArchivedManga *manga = new ArchivedManga(file);
		manga->UID = BuildUID(manga->Name);

		LOG("Add comic book: %s", manga->Name.ToCStr());

		MangaWrapper *wrapper = new MangaWrapper(manga);
		wrapper->SetThumb(manga->GetThumb());
		wrapper->Name = manga->Name;
		_Mangas.PushBack(wrapper);
	}

	// Interprets a directory
	// a) has folders -> directory is a manga with sub mangas
	// b) has no folders but images -> directory is a manga
	// *) 
	void LocalMangaProvider::ExploreDirectory(String dir) {
		// Create file list here already, to prevent doing it twice
		Array<String> files = DirectoryFileList(dir.ToCStr());
		LOG("Exploring directory: %s", dir.ToCStr());
		
		LocalScanResult scan = ScanDirectory(dir, files);

		LOG("Has Images: %d, Has Folders: %d, #Comic Books: %d", scan.HasImages, scan.HasFolders, scan.Archives.GetSizeI());

		// Read comic books
		for (int i = 0; i < scan.Archives.GetSizeI(); i++) {
			AddArchivedManga(scan.Archives[i]);
		}

		bool isManga = scan.HasImages && !scan.HasFolders;
		if (scan.HasFolders) {
			// This directory contains more than one manga, so add it as a provider

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
				// Give manga a second chance
				isManga = scan.HasImages;
			}
		}

		if (isManga) {
			// This directory *is* a manga, add it to the provider
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
	}

	LocalScanResult LocalMangaProvider::ScanDirectory(String dir, const Array<String> &files) {
		LocalScanResult result;

		LOG("Exploring directory: %s", dir.ToCStr());

		// Try to parse as manga
		for (int i = 0; i < files.GetSizeI(); i++) {
			if (files[i].Right(1) == "/") {
				// Found a container (folder)!
				result.HasFolders = true;
			}

			if (IsSupportedExt(files[i].GetExtension())) {
				result.HasImages = true;
			}

			if (IsArchivedManga(files[i].GetExtension())) {
				result.Archives.PushBack(files[i]);
			}
		}

		return result;
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