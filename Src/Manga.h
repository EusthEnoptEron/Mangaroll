#pragma once

#include "App.h"
#include "GlObject.h"
#include "Page.h"
#include "AsyncTexture.h"

using namespace OVR;

namespace OvrMangaroll {

	class Manga : public GlObject
	{
	public:
		Manga(void);
		~Manga(void);
		void Draw(const Matrix4f&);
		void AddPage(Page *page);
		void Update(float angle, bool onlyVisual = false);
		int GetProgress(void);
		void SetProgress(int page);
		int GetCount(void);
		Page &GetPage(int);
		String Name;
		bool Selectionable;
		AsyncTexture *GetCover();
		void Unload();
	private:
		GlProgram _Prog;
		int _Count;
		Page *_First;
		Page *_Last;
		Page *_Selection;
		int _SelectionIndex;
		float _Angle;
		int _Progress;
		AsyncTexture *_Cover;
	};

}
