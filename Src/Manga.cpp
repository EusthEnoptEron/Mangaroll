#include "Manga.h"
#include "Kernel\OVR_JSON.h"
#include "Helpers.h"
#include "Web.h"
#include "jni.h"
#include "Android\JniUtils.h"
#include "Config.h"

#ifndef ANDROID
#define ANDROID
#endif

namespace OvrMangaroll {


		Manga::Manga(void) 
			: GlObject()
			, Name()
			, _Initialized(false)
			, _Count(0)
			, _First(NULL)
			, _Selection(NULL)
			, _SelectionIndex(0)
			, _Progress(0)
			, _AngleOffset(0)
			, _LastSetPage(0)
	{
			WARN("YAY 2");
	}

		Manga::~Manga(void)
	{
	}

	void Manga::Draw(const Matrix4f &m) {
		if(_First != NULL) {
			Matrix4f mat = m * Mat;
			Page *ref = _First;
			do {
				ref->Draw(mat);
			} while( (ref = ref->GetNext()) != NULL);  
		}
	}


	void Manga::AddPage(Page *page) {
		if(_First == NULL) {
			// First page
			_First = page;
			_Last  = page;
			page->SetOffset(0);
		} else {

			_Last->SetNext(page);
			page->SetPrev(_Last);

			_Last = page;
		}
		_Count++;
	}

	int Manga::GetProgress(void) {
		return _SelectionIndex;
	}

	void Manga::SetProgress(int page, float angleToAppear) {
		Page *activePage = NULL;
		Page *p = _First;

		for (int i = 0; i < _Count; i++) {
			if (page == i) {
				activePage = p;
			}
			p->Reset();
			p = p->GetNext();
		}

		if (activePage == NULL) {
			WARN("INVALID PAGE");
			activePage = _First;
		}

		// Make the current angle to 0
		if (angleToAppear == FLT_MAX) {
			angleToAppear = _Angle;
		}

		_AngleOffset = -(angleToAppear - 20);
		_LastSetPage = page;

		ApplyAngleOffset();

		if (activePage != NULL) {
			activePage->SetOffset(0);
		}
	}

	Page &Manga::GetPage(int pageNo) {
		if (pageNo < 0) return *_First;
		if (pageNo >= _Count) return *_Last;

		Page *p = _First;
		for (int i = 0; i < pageNo; i++) {
			p = p->GetNext();
		}

		return *p;
	}

	int Manga::GetCount(void) {
		return _Count;
	
	}
	void Manga::Unload() {
		Page *p = _First;
		for (int i = 0; i < _Count; i++) {
			p->Reset();
			p = p->GetNext();
		}
	}

	void Manga::IncreaseAngleOffset(float deg) {
		_AngleOffset += deg;
		ApplyAngleOffset();
	}

	void Manga::ApplyAngleOffset() {
		float rotation = AppState::Conf->LeftToRight ? _AngleOffset + 180 : -_AngleOffset;
		this->Rotation = Quatf(Vector3f(0, 1, 0), rotation / 180.0f * Mathf::Pi);
		this->Touch();
	}

	void Manga::Update(float angle, bool onlyVisual) {
		_Angle = angle;
		angle += _AngleOffset;
		UpdateModel();

		Page *ref = _First;
		bool electionFinished = false;
		Page *selectionCandidate = NULL;
		int selectionIndex = 0;

		int i = 0;
		if(_First != NULL) {
			do {
				ref->Update(angle, onlyVisual);
				if (Selectionable && !electionFinished) {
					if (ref->IsTarget(angle)) {
						selectionCandidate = ref;
						selectionIndex = i;

						if (_Selection == ref) {
							electionFinished = true; // Prioritize
						}
						
					}
				}

				i++;
			} while( (ref = ref->GetNext()) != NULL);
		}


		// Update selection (if anything changed)
		if (selectionCandidate != _Selection) {
			if (_Selection != NULL) {
				_Selection->SetSelected(false);
			}
			_Selection = selectionCandidate;

			if (Selectionable) {
				// Because it's needed for progress
				_SelectionIndex = selectionIndex;
			}
			if (_Selection != NULL) {
				_Selection->SetSelected(true);
			}

			if (_Selection == NULL) {
				WARN("SELECTION: NONE");
			}
			else {
				WARN("SELECTION: %s", _Selection->GetPath().ToCStr());
			}
		}

		// Internal update
		_Update();
	}


	// #################### REMOTE MANGA #######################
	RemoteManga::RemoteManga()
		: Manga()
		, ID("")
		, FetchUrl("")
		, _Payload()
		, _Loading(false)
		, _HasMore(true)
		, _Page(0)
	{

	}

	void RemoteManga::_Init() {
		Fetch();
		
	}
	void RemoteManga::Fetch() {
		_Loading = true;
		_HasMore = false;

		String url = ParamString::InsertParam(FetchUrl.ToCStr(), ParamString::PARAM_PAGE, _Page);
		url = ParamString::InsertParam(url.ToCStr(), ParamString::PARAM_ID, ID.ToCStr());

		Web::Download(
			url,
			RemoteManga::OnDownload,
			this
		);
	}

	void RemoteManga::_Update() {
		// Clear payload
		if (!_Loading && _Payload.GetSizeI() > 0) {
			// were we already able to set page?
			bool setPage = true;
			if (_Count > _LastSetPage) {
				setPage = false;
			}

			for (int i = 0; i < _Payload.GetSizeI(); i++) {
				AddPage(_Payload[i]);
			}
			_Payload.Clear();

			if (setPage && _Count > _LastSetPage) {
				// We can now set the page!
				SetProgress(_LastSetPage);
			}
		}

		if (!_Loading && _HasMore && _SelectionIndex > _Count - 3) {
			Fetch();
		}
	}

	void RemoteManga::OnDownload(void *buffer, int length, void *p) {
		
		RemoteManga *self = (RemoteManga *)p;

		JSON *file = JSON::Parse((const char *)buffer);
		if (file != NULL) {
			JsonReader resReader(file);
			if (resReader.IsValid() && resReader.IsObject() && resReader.GetChildBoolByName("success")) {
				self->_HasMore = resReader.GetChildBoolByName("hasMore");
				JsonReader pageReader(resReader.GetChildByName("images"));
				if (pageReader.IsValid() && pageReader.IsArray()) {
					while (!pageReader.IsEndOfArray()) {
						Page *p = new Page(pageReader.GetNextArrayString());
						self->_Payload.PushBack(p);
					}
				}
			}
		}

		self->_Page++;
		delete file;
		self->_Loading = false;
	}

	//########### DYNAMIC MANGA ##########
	DynamicManga::DynamicManga(JNIEnv *env, jobject obj)
		: Manga()
		, ID("")
		, _Payload()
		, _Loading(false)
		, _HasMore(true)
		, _Fetcher(env, obj)
	{

	}

	void DynamicManga::_Init() {
		Fetch();

	}
	void DynamicManga::Fetch() {
		_Loading = true;
		_HasMore = false;

		AppState::Scheduler->Schedule(FetchList, this);
	}

	void DynamicManga::_Update() {
		// Clear payload
		if (!_Loading && _Payload.GetSizeI() > 0) {
			// were we already able to set page?
			bool setPage = true;
			if (_Count > _LastSetPage) {
				setPage = false;
			}

			for (int i = 0; i < _Payload.GetSizeI(); i++) {
				AddPage(_Payload[i]);
			}
			_Payload.Clear();

			if (setPage && _Count > _LastSetPage) {
				// We can now set the page!
				SetProgress(_LastSetPage);
			}
		}

		if (!_Loading && _HasMore && _SelectionIndex > _Count - 3) {
			Fetch();
		}
	}

	void DynamicManga::FetchList(JNIThread *t, void *p) {

		DynamicManga *self = (DynamicManga *)p;
		JavaGlobalObject &fetcher = self->_Fetcher;
		JNIEnv *env = t->Env;

		jmethodID fetch = env->GetMethodID(AppState::MangaFetcherClass, "fetch", "()[Ljava/lang/String;");
		jmethodID hasMore = env->GetMethodID(AppState::FetcherClass, "hasMore", "()Z");

		jobjectArray resultArray = (jobjectArray)(env->CallObjectMethod(fetcher.GetJObject(), fetch));

		if (resultArray != NULL) {
			int count = env->GetArrayLength(resultArray);
			for (int i = 0; i < count; i++) {
				String imgUrl = JavaUTFChars(env, (jstring)env->GetObjectArrayElement(resultArray, i)).ToStr();
				Page *p = new Page(imgUrl);
				self->_Payload.PushBack(p);
			}
			env->DeleteLocalRef(resultArray);
			
			self->_HasMore = env->CallBooleanMethod(fetcher.GetJObject(), hasMore);
		}

		self->_Loading = false;
	}

	// ################# ArchivedManga ##############
	ArchivedManga::ArchivedManga(String filePath)
		: Manga()
		, _Path(filePath)
		, _Valid(true)
	{
		Populate();
	}


	void ArchivedManga::Populate() {
		_ArchiveType = IsZIP(_Path.GetExtension()) ? ARCHIVE_ZIP : ARCHIVE_RAR;
		String protocol = _ArchiveType == ARCHIVE_ZIP ? "zip" : "rar";

		const ovrJava *java = AppState::Instance->GetJava();
		JNIEnv* env = java->Env;

		JavaClass clazz(env, env->GetObjectClass(java->ActivityObject));
		JavaString pathArg(env, _Path.ToCStr());

		jmethodID getFileList = env->GetStaticMethodID(clazz.GetJClass(), _ArchiveType == ARCHIVE_ZIP ? "GetZipFileList" : "GetRarFileList", "(Ljava/lang/String;)[Ljava/lang/String;");
		jmethodID getName = env->GetStaticMethodID(clazz.GetJClass(), _ArchiveType == ARCHIVE_ZIP ? "GetZipName" : "GetRarName", "(Ljava/lang/String;)Ljava/lang/String;");
		jobjectArray fileList = (jobjectArray)(env->CallStaticObjectMethod(clazz.GetJClass(), getFileList, pathArg.GetJString()));

		if (fileList != NULL) {
			JavaUTFChars name = JavaUTFChars(env, (jstring)(env->CallStaticObjectMethod(clazz.GetJClass(), getName, pathArg.GetJString())));
			Name = name.ToStr();

			int length = env->GetArrayLength(fileList);
			for (int i = 0; i < length; i++) {
				JavaUTFChars file(env, (jstring)env->GetObjectArrayElement(fileList, i));

				// Build page with custom protocol
				Page *page = new Page(protocol + "://" + _Path + "|" + file);
				AddPage(page);
			}

			env->DeleteLocalRef(fileList);
		}
		else {
			WARN("Could not open archive! %s", _Path.ToCStr());
			_Valid = false;
		}
	}

	MangaWrapper::MangaWrapper(MangaProvider *provider)
		: _Manga(NULL)
		, _Category(provider)
		, _Cover(NULL)
	{
	}

	MangaWrapper::MangaWrapper(Manga *manga)
		: _Manga(manga)
		, _Category(NULL)
		, _Cover(NULL)
	{
	}

	void MangaWrapper::SetThumb(String thumb) {
		if (_Cover != NULL) {
			delete _Cover;
		}
		_Cover = new AsyncTexture(thumb, 3);
		_Cover->MaxHeight = 400;
	}

}
