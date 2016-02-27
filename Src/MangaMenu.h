#pragma once

#include "VRMenu.h"
#include "UI\UIMenu.h"

using namespace OVR;

namespace OvrMangaroll {

	// Forward declaration
	class Mangaroll;

	class MangaMenu : public UIMenu {
	public:
		MangaMenu(OvrGuiSys & guiSys, Mangaroll *app) : UIMenu(guiSys)
			, _App(app) {
		}
		
	private:
		Mangaroll *_App;

	};
}