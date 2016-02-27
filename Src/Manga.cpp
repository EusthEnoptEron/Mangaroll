#include "Manga.h"

namespace OvrMangaroll {


		Manga::Manga(void) 
			: GlObject(),
			_First(NULL), 
			_Selection(NULL)
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
	}

	void Manga::Update(float angle) {
		UpdateModel();

		Page *ref = _First;
		bool electionFinished = false;
		Page *selectionCandidate = NULL;

		if(_First != NULL) {
			do {
				ref->Update(angle);
				if(!electionFinished && ref->IsTarget(angle)) {
					if(_Selection == ref) {
						selectionCandidate = ref;
						electionFinished = true; // Prioritize
					} else {
						selectionCandidate = ref;
					}
				}
			} while( (ref = ref->GetNext()) != NULL);
		}

		// Update selection (if anything changed)
		if(selectionCandidate != _Selection) {
			if(_Selection != NULL) {
				_Selection->SetSelected(false);
			}
			_Selection = selectionCandidate;
			if(_Selection != NULL) {
				_Selection->SetSelected(true);
			}
		}
	}
}
