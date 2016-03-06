#pragma once

#include "View.h"
#include "UI\UIMenu.h"
#include "UI\UIContainer.h"
#include "UI\UILabel.h"
#include "UI\UIMenu.h"
#include "UI\UITexture.h"
#include "UI\UIImage.h"
#include "VRMenuComponent.h"
#include "AsyncTexture.h"

using namespace OVR;
namespace OvrMangaroll {

	// Forward declaration
	class Mangaroll;
	class Manga;
	class MangaSelectorComponent;


	class MangaPanelComponent : public VRMenuComponent {
	public:
		MangaPanelComponent(OvrGuiSys &guiSys);
		virtual ~MangaPanelComponent() { }
		void AddToMenu(UIMenu *menu, UIObject *parent = NULL);
		MangaSelectorComponent *Selector;
		Manga *_Manga;
	private:
		virtual eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);
		UIMenu *_Menu;
		UIObject *_Parent;
		OvrGuiSys &_Gui;
		bool _Focused;
		bool _Touched;

	};

	class MangaPanel : public UIObject {
	public:
		MangaPanel(OvrGuiSys &guiSys);
		virtual ~MangaPanel();
		void SetSelector(MangaSelectorComponent *selector);

		void AddToMenu(UIMenu *menu, UIObject *parent = NULL);
		void SetManga(Manga *manga);
		Manga *GetManga();
		float Position;
		int Index;
	private:
		MangaSelectorComponent *_Selector;
		Manga *_Manga;
		void Init(void);
		UIImage *_Background;
		UILabel *_Title;
		UIImage *_Cover;
		UITexture *_CoverTexture;
		MangaPanelComponent *_Component;
	};


	class MangaSelectorComponent : public VRMenuComponent {
	public:
		MangaSelectorComponent(OvrGuiSys &guiSys);
		virtual ~MangaSelectorComponent() { }
		void AddManga(Manga &manga);
		void AddToMenu(UIMenu *menu, UIObject *parent);

		void SetOnSelectManga(void(*cb)(Manga*, void*), void* obj) {
			this->_Callback = cb;
			this->_CallbackObject = obj;
		}

		void SelectManga(Manga *manga);
		static int const NUM_PANELS = 10;
		// Number of visible panels on either side
		static int const NUM_VISIBLE_PANELS = 2;
	private:
		virtual eMsgStatus OnEvent_Impl(OvrGuiSys & guiSys, VrFrame const & vrFrame,
			VRMenuObject * self, VRMenuEvent const & event);

		void Update(void);
		void RearrangePanels(void);
		void UpdatePositions(void);
		void OnTouchDown(VrInput input);
		void OnTouchUp(VrInput input);
		void OnTouchMove(VrInput input);

		Array<Manga *> _Mangas;
		UIMenu *_Menu;
		UIObject *_Parent;
		UIContainer *_Container;
		OvrGuiSys &_Gui;
		void(*_Callback)(Manga*, void*);
		void *_CallbackObject;
		Array<MangaPanel*> _Panels;
		Array<MangaPanel*> _UsedPanels;
		Array<MangaPanelComponent*> _PanelComponents;
		float _Index;
		float _Speed;
		float _Gravity;
		Vector2f _LastTouch;
		float _TouchStartTime;
		bool _IsTouching;
		float _PanelHeight;
	};

	class MangaSelectionView : public View {
	public:
		MangaSelectionView(Mangaroll &app);
		virtual ~MangaSelectionView();

		virtual void 		OneTimeInit(const char * launchIntent);
		virtual void		OneTimeShutdown();

		virtual void 		OnOpen();
		virtual void 		OnClose();
		virtual bool 		OnKeyEvent(const int keyCode, const int repeatCount, const KeyEventType eventType);
		virtual Matrix4f 	Frame(const VrFrame & vrFrame);
		virtual Matrix4f	GetEyeViewMatrix(const int eye) const;
		virtual Matrix4f	GetEyeProjectionMatrix(const int eye, const float fovDegreesX, const float fovDegreesY) const;
		virtual Matrix4f 	DrawEyeView(const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms);

		void OnSelectManga(Manga *);

	private:
		Mangaroll &_Mangaroll;
		UIMenu *_Menu;

		UIContainer *_MainContainer;
		UILabel *_LocalSrcLabel;
		UILabel *_RemoteSrcLabel;
		MangaSelectorComponent *_Selector;
		UIContainer *_SelectorContainer;
		AsyncTexture *_AsyncTex;
	};


}