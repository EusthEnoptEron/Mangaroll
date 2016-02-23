#include "PageCarousel.h"

namespace OvrMangaroll {
		static const char VERTEX_SHADER[] =
		#if defined( OVR_OS_ANDROID )
			"#version 300 es\n"
		#endif
			"in vec3 Position;\n"
			"in vec4 VertexColor;\n"
			"in vec2 TexCoord;\n"
			"uniform mat4 Viewm;\n"
			"uniform mat4 Projectionm;\n"
			"uniform mat4 Modelm;\n"
			"out vec4 fragmentColor;\n"
			"out vec2 oTexCoord;\n"
			"void main()\n"
			"{\n"
			"	gl_Position = Projectionm * Viewm * Modelm * vec4( Position, 1.0 );\n"
			"	fragmentColor = VertexColor;\n"
			"   oTexCoord = TexCoord;\n"
			"}\n";

		static const char FRAGMENT_SHADER[] =
		#if defined( OVR_OS_ANDROID )
			"#version 300 es\n"
		#endif
			"in lowp vec4 fragmentColor;\n"
			"uniform sampler2D Texture0;\n"
			"uniform int IsSelected;\n"
			"in lowp vec2 oTexCoord;\n"
			"out lowp vec4 outColor;\n"
			"void main()\n"
			"{\n"
			"	outColor = IsSelected == 1 ? fragmentColor : texture( Texture0, oTexCoord );\n"
			"	//outColor = fragmentColor;\n"
			"}\n";


	PageCarousel::PageCarousel(void) : GlObject(), _First(NULL), _Selection(NULL)
	{
		_Prog = BuildProgram(VERTEX_SHADER, FRAGMENT_SHADER);
	}

	PageCarousel::~PageCarousel(void)
	{
		DeleteProgram(_Prog);
	}

	void PageCarousel::Draw(const Matrix4f &m) {
		if(_First != NULL) {
			Matrix4f mat = m * Mat;
			Page *ref = _First;
			do {
				ref->Draw(mat);
			} while( (ref = ref->GetNext()) != NULL);  
		}
	}

	void PageCarousel::AddPage(Page *page) {
		if(_First == NULL) {
			_First = page;
			_Last  = page;

			page->SetOffset(0);
		} else {

			_Last->SetNext(page);
			_Last = page;
		}

		page->SetProgram(_Prog);
	}

	void PageCarousel::Update(float angle) {
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
