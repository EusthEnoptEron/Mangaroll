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
			_First = page;
			_Last  = page;

			page->SetOffset(0);
		} else {

			_Last->SetNext(page);
			_Last = page;
		}
		_Count++;
	}

	int Manga::GetProgress(void) {
		return _SelectionIndex;
	}

	int Manga::GetCount(void) {
		return _Count;
	}

	void Manga::Update(float angle) {
		UpdateModel();

		Page *ref = _First;
		bool electionFinished = false;
		Page *selectionCandidate = NULL;
		int selectionIndex = 0;

		int i = 0;
		if(_First != NULL) {
			do {
				ref->Update(angle);
				if(!electionFinished && ref->IsTarget(angle)) {
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
