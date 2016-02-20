#ifndef OVRPAGE_H
#define OVRPAGE_H

#include "App.h"

using namespace OVR;
namespace OvrMangaroll {
	enum LoadState { UNLOADED, LOADING, LOADED };
	enum DisplayState { VISIBLE, INVISIBLE, LIMBO };

	class Page {
	public:
		Page(String _path) : _Path(_path), _Offset(0), _LoadState(UNLOADED), _DisplayState(INVISIBLE), _Next(__nullptr) {
			
		};

		~Page();

		void Update(float angle);
		bool NeedsDrawing();
		void SetNext(Page *next);
		Page &GetNext();

	private:
		String _Path;
		int _Offset;
		LoadState _LoadState;
		DisplayState _DisplayState;
		Page *_Next;
		

	protected:

	};
}

#endif