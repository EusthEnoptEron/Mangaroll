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
		void Draw(const Matrix4f &m);
		void AddPage(Page *page);
		void Update(float angle);
	private:
		GlProgram _Prog;
		int _Count;
		Page *_First;
		Page *_Last;
	};

}
