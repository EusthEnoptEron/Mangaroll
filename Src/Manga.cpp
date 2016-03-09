#include "Manga.h"
#include "Kernel\OVR_JSON.h"
#include "Helpers.h"
#include "Web.h"

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
			, _Cover(NULL)
			, _AngleOffset(0)
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

	AsyncTexture *Manga::GetCover() {
		return _Cover;
	}

	void Manga::AddPage(Page *page) {
		if (_Cover == NULL) {
			_Cover = new AsyncTexture(page->GetPath(), 3);
			_Cover->MaxHeight = 400;
		}

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

	void Manga::SetProgress(int page) {
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
		_AngleOffset = -_Angle;
		this->Rotation = Quatf(Vector3f(0, 1, 0), _Angle / 180.0f * Mathf::Pi);
		this->Touch();

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
				if (Selectionable && !electionFinished && ref->IsTarget(angle)) {
					selectionCandidate = ref;
					selectionIndex = i;
					if(_Selection == ref) {
						electionFinished = true; // Prioritize
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

	RemoteManga::RemoteManga()
		: Manga()
		, ID(0)
		, FetchUrl("")
		, _Payload()
		, _Loading(false)
	 {

	}

	void RemoteManga::SetThumb(String thumb) {
		if (_Cover != NULL) {
			delete _Cover;
		}
		_Cover = new AsyncTexture(thumb, 3);
	}


	void RemoteManga::_Init() {
		_Loading = true;
		Web::Download(
			String::Format(FetchUrl.ToCStr(), ID),
			RemoteManga::OnDownload,
			this
		);
	}
	void RemoteManga::_Update() {
		if (!_Loading && _Payload.GetSizeI() > 0) {
			for (int i = 0; i < _Payload.GetSizeI(); i++) {
				AddPage(_Payload[i]);
			}
			_Payload.Clear();
		}
	}

	void RemoteManga::OnDownload(void *buffer, int length, void *p) {
		
		RemoteManga *self = (RemoteManga *)p;
		
		JSON *file = JSON::Parse((const char *)buffer);
		if (file != NULL) {
			JsonReader resReader(file);
			if (resReader.IsValid() && resReader.IsObject() && resReader.GetChildBoolByName("success")) {
				JsonReader pageReader(resReader.GetChildByName("images"));
				if (pageReader.IsValid() && pageReader.IsArray()) {
					while (!pageReader.IsEndOfArray()) {
						Page *p = new Page(pageReader.GetNextArrayString());
						self->_Payload.PushBack(p);
					}
				}
			}
		}

		delete file;
		self->_Loading = false;
	}

}
