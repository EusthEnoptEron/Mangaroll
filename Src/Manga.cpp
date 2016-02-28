#include "Manga.h"

namespace OvrMangaroll {


		Manga::Manga(void) 
			: GlObject()
			, Name()
			, _Count(0)
			, _First(NULL)
			, _Selection(NULL)
			, _SelectionIndex(0)
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

	void Manga::SetProgress(int page) {
		Page *activePage;
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

		activePage->SetOffset(_Angle * PIXELS_PER_DEGREE);
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

	void Manga::Update(float angle, bool onlyVisual) {
		_Angle = angle;
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
		if(selectionCandidate != _Selection) {
			if(_Selection != NULL) {
				_Selection->SetSelected(false);
			}
			_Selection = selectionCandidate;
			_SelectionIndex = selectionIndex;
			if(_Selection != NULL) {
				_Selection->SetSelected(true);
			}
		}
	}
}
