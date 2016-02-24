#pragma once

#include "App.h"
#include "GlObject.h"
#include "Page.h"

using namespace OVR;

namespace OvrMangaroll {

	class PageCarousel : public GlObject
	{
	public:
		PageCarousel(void);
		~PageCarousel(void);
		void Render(const Matrix4f&, const Matrix4f&, const Matrix4f&, const Matrix4f&);
		void Draw(const Matrix4f&);
		void AddPage(Page *page);
		void Update(float angle);
	private:
		GlProgram _Prog;
		int _Count;
		Page *_First;
		Page *_Last;
		Page *_Selection;
	};

}
